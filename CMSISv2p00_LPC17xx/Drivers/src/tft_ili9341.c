/**
 * @file tft_ili9341.c
 * @brief Implementación del driver TFT ILI9341 para LPC1769 via SSP0
 *
 * Capas:
 *  1. HAL SSP0       		– registros LPC_SSP0 del CMSIS LPC1769
 *  2. ILI9341        		– comandos del controlador, inicialización, ventana de dirección
 *  3. Primitive functions 	– drawPixel, líneas, rectángulos, círculos, texto
 */

#include "LPC17xx.h"   /* CMSIS — ajustar path según tu toolchain */
#include <stdlib.h>    /* abs() */
#include "tft_ili9341.h"

/* =========================================================
 *  Macros de GPIO
 * ========================================================= */

#define GPIO_SET(port, pin)   ((port)->FIOSET = (1u << (pin)))
#define GPIO_CLR(port, pin)   ((port)->FIOCLR = (1u << (pin)))
#define GPIO_OUT(port, pin)   ((port)->FIODIR |= (1u << (pin)))

#define TFT_CS_HIGH()   GPIO_SET(TFT_CS_PORT,  TFT_CS_PIN)
#define TFT_CS_LOW()    GPIO_CLR(TFT_CS_PORT,  TFT_CS_PIN)
#define TFT_DC_HIGH()   GPIO_SET(TFT_DC_PORT,  TFT_DC_PIN)   /* datos */
#define TFT_DC_LOW()    GPIO_CLR(TFT_DC_PORT,  TFT_DC_PIN)   /* comando */
#define TFT_RST_HIGH()  GPIO_SET(TFT_RST_PORT, TFT_RST_PIN)
#define TFT_RST_LOW()   GPIO_CLR(TFT_RST_PORT, TFT_RST_PIN)

/* =========================================================
 *  Comandos ILI9341
 * ========================================================= */

#define ILI9341_SWRESET     0x01
#define ILI9341_SLPOUT      0x11
#define ILI9341_DISPOFF     0x28
#define ILI9341_DISPON      0x29
#define ILI9341_CASET       0x2A   /* Column Address Set */
#define ILI9341_PASET       0x2B   /* Page (Row) Address Set */
#define ILI9341_RAMWR       0x2C   /* Memory Write */
#define ILI9341_MADCTL      0x36   /* Memory Access Control (rotación) */
#define ILI9341_COLMOD      0x3A   /* Pixel Format */
#define ILI9341_FRMCTR1     0xB1
#define ILI9341_DFUNCTR     0xB6
#define ILI9341_PWCTR1      0xC0
#define ILI9341_PWCTR2      0xC1
#define ILI9341_VMCTR1      0xC5
#define ILI9341_VMCTR2      0xC7
#define ILI9341_GMCTRP1     0xE0
#define ILI9341_GMCTRN1     0xE1
#define ILI9341_IFCTL       0xF6

/* Bits MADCTL para rotación */
#define MADCTL_MY   0x80
#define MADCTL_MX   0x40
#define MADCTL_MV   0x20
#define MADCTL_BGR  0x08

/* =========================================================
 *  Estado interno del driver
 * ========================================================= */

static uint16_t _width  = TFT_WIDTH;
static uint16_t _height = TFT_HEIGHT;
static tft_rotation_t _rotation = TFT_ROT_0;

/* =========================================================
 *  FUENTE 5×7 px  (ASCII 32 – 126)
 *  Cada columna es un byte; bit 0 = fila superior
 * ========================================================= */
const uint8_t tft_font5x7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* '\'' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x10,0x08,0x08,0x10,0x08}, /* '~' */
};

/* =========================================================
 *  Delay simple (busy-wait)
 *  Calibrado para ~100 MHz; ajustar si se usa otra frecuencia
 * ========================================================= */

void delay_ms(uint32_t ms) {
    /* ~100000 ciclos por ms a 100 MHz */
    volatile uint32_t count = ms * 10000u;
    while (count--) { __asm volatile("nop"); }
}

/* =========================================================
 *  HAL — SSP0
 * ========================================================= */

/**
 * @brief Inicializa SSP0 en modo SPI Master, CPOL=0, CPHA=0, 8-bit.
 *        Velocidad: PCLK / (CPSDVSR * (SCR+1))
 *        Ejemplo: 60 MHz / (2 * 3) = 10 MHz
 */
static void ssp0_init(void) {

    // 1. Alimentar SSP0 (PCONP bit 21)
    LPC_SC->PCONP |= (1u << 21);

    // 2. PCLK_SSP0 = CCLK (bits 21:20 de PCLKSEL1 = 01)
    LPC_SC->PCLKSEL1 &= ~(3u << 20);
    LPC_SC->PCLKSEL1 |=  (1u << 20);   // PCLK = CCLK → CCLK = 100 MHz

    // 3. Pinsel P0.15(SCK), P0.17(MISO), P0.18(MOSI) → función SSP0
    LPC_PINCON->PINSEL0 &= ~(3u << 30);
    LPC_PINCON->PINSEL0 |=  (2u << 30); // P0.15 SCK0

    LPC_PINCON->PINSEL1 &= ~((3u << 2) | (3u << 4));
    LPC_PINCON->PINSEL1 |=  ((2u << 2) | (2u << 4)); // P0.17 MISO0, P0.18 MOSI0

    /* 4. Configurar SSP0 */
    LPC_SSP0->CR0  = (7u << 0)   /* DSS: 8-bit */
                   | (0u << 4)   /* FRF: SPI */
                   | (0u << 6)   /* CPOL: idle LOW */
                   | (0u << 7)   /* CPHA: captura en flanco 1 */
                   | (2u << 8);  /* SCR: divisor adicional → 20 MHz */

    LPC_SSP0->CPSR = 2u;         /* CPSDVSR = 2  (debe ser par, ≥2) */

    /* 5. Habilitar SSP0 como Master */
    LPC_SSP0->CR1  = (1u << 1);  /* SSE=1, MS=0 (master) */
}

/**
 * @brief Envía un byte por SSP0 (bloqueante).
 */
static inline void ssp0_send(uint8_t byte) {

    while (!(LPC_SSP0->SR & (1u << 1))){};  /* espera TNF (TX FIFO no lleno) */
    LPC_SSP0->DR = byte;
    while (LPC_SSP0->SR & (1u << 2))     /* drena RX FIFO */
        (void)LPC_SSP0->DR;
}

/** @brief Limpia lo que termino de recibir
 */
static inline void ssp0_flush(void)
{
    while (LPC_SSP0->SR & (1u << 4)) {};    /* espera BSY=0 */
    while (LPC_SSP0->SR & (1u << 2))     /* drena resto de RX */
        (void)LPC_SSP0->DR;
}

/**
 * @brief Envía dos bytes (word RGB565).
 */
static inline void ssp0_send16(uint16_t word) {

    ssp0_send((uint8_t)(word >> 8));
    ssp0_send((uint8_t)(word & 0xFF));
}

/* =========================================================
 *  Inicialización GPIO de control
 * ========================================================= */

static void gpio_ctrl_init(void) {

    GPIO_OUT(TFT_CS_PORT,  TFT_CS_PIN);
    GPIO_OUT(TFT_DC_PORT,  TFT_DC_PIN);
    GPIO_OUT(TFT_RST_PORT, TFT_RST_PIN);
    ssp0_flush();
    TFT_CS_HIGH();
    TFT_DC_HIGH();
    TFT_RST_HIGH();
}

/* =========================================================
 *  ILI9341 — envío de comandos y datos
 * ========================================================= */

static void ili_cmd(uint8_t cmd) {

    TFT_DC_LOW();
    TFT_CS_LOW();
    ssp0_send(cmd);
    ssp0_flush();
    TFT_CS_HIGH();
}

static void ili_data(uint8_t data) {

    TFT_DC_HIGH();
    TFT_CS_LOW();
    ssp0_send(data);
    ssp0_flush();
    TFT_CS_HIGH();
}

static void ili_cmd_data(uint8_t cmd, const uint8_t *buf, uint8_t len) {

    ili_cmd(cmd);
    TFT_DC_HIGH();
    TFT_CS_LOW();
    for (uint8_t i = 0; i < len; i++) {
        ssp0_send(buf[i]);
    }
    ssp0_flush();
    TFT_CS_HIGH();
}

/* =========================================================
 *  Secuencia de inicialización ILI9341
 * ========================================================= */

static void ili9341_hw_init(void) {

    /* Reset hardware */
    TFT_RST_LOW();
    delay_ms(5);
    TFT_RST_HIGH();
    delay_ms(5);

    /* Software reset */
    ili_cmd(ILI9341_SWRESET);
    delay_ms(5);

    /* salir de sleep */
    ili_cmd(ILI9341_SLPOUT);
    delay_ms(5);

    /* Pixel format: 16 bits/pixel (RGB565) */
    uint8_t colmod = 0x55;
    ili_cmd_data(ILI9341_COLMOD, &colmod, 1);

    /* Frame rate: ~70 Hz */
    {
        uint8_t d[] = {0x00, 0x1B};
        ili_cmd_data(ILI9341_FRMCTR1, d, 2);
    }

    /* Power control */
    { uint8_t d[] = {0x23};           ili_cmd_data(ILI9341_PWCTR1, d, 1); }
    { uint8_t d[] = {0x10};           ili_cmd_data(ILI9341_PWCTR2, d, 1); }
    { uint8_t d[] = {0x3E, 0x28};     ili_cmd_data(ILI9341_VMCTR1, d, 2); }
    { uint8_t d[] = {0x86};           ili_cmd_data(ILI9341_VMCTR2, d, 1); }

    /* Interface control: little-endian, RGB */
    { uint8_t d[] = {0x01, 0x30, 0x00}; ili_cmd_data(ILI9341_IFCTL, d, 3); }

    /* Gamma positiva */
    {
        uint8_t d[] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,
                        0x37,0x07,0x10,0x03,0x0E,0x09,0x00};
        ili_cmd_data(ILI9341_GMCTRP1, d, 15);
    }
    /* Gamma negativa */
    {
        uint8_t d[] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,
                        0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F};
        ili_cmd_data(ILI9341_GMCTRN1, d, 15);
    }

    ili_cmd(ILI9341_DISPON);    /* encender pantalla */
}

/* =========================================================
 *  API pública — Inicialización y rotación
 * ========================================================= */

void tft_set_rotation(tft_rotation_t rot) {

    _rotation = rot;
    uint8_t madctl = MADCTL_BGR;

    switch (rot) {
        case TFT_ROT_0:
            madctl |= MADCTL_MX;
            _width  = TFT_WIDTH;
            _height = TFT_HEIGHT;
            break;
        case TFT_ROT_90:
            madctl |= (MADCTL_MV | MADCTL_MX | MADCTL_MY);
            _width  = TFT_HEIGHT;
            _height = TFT_WIDTH;
            break;
        case TFT_ROT_180:
            madctl |= MADCTL_MY;
            _width  = TFT_WIDTH;
            _height = TFT_HEIGHT;
            break;
        case TFT_ROT_270:
            madctl |= MADCTL_MV;
            _width  = TFT_HEIGHT;
            _height = TFT_WIDTH;
            break;
    }
    ili_cmd_data(ILI9341_MADCTL, &madctl, 1);
}

void tft_init(tft_rotation_t rot) {

    gpio_ctrl_init();
    ssp0_init();
    ili9341_hw_init();
    tft_set_rotation(rot);
}

uint16_t tft_width(void)  { return _width;  }
uint16_t tft_height(void) { return _height; }

/* =========================================================
 *  API pública — Primitivas de bajo nivel
 * ========================================================= */

void tft_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

    /* Column Address Set */
    uint8_t caset[4] = {
        (uint8_t)(x0 >> 8), (uint8_t)(x0),
        (uint8_t)(x1 >> 8), (uint8_t)(x1)
    };
    ili_cmd_data(ILI9341_CASET, caset, 4);

    /* Page Address Set */
    uint8_t paset[4] = {
        (uint8_t)(y0 >> 8), (uint8_t)(y0),
        (uint8_t)(y1 >> 8), (uint8_t)(y1)
    };
    ili_cmd_data(ILI9341_PASET, paset, 4);

    /* Memory Write — habilita escritura continua */
    ili_cmd(ILI9341_RAMWR);
    TFT_DC_HIGH();
    TFT_CS_LOW();
    /* CS queda LOW; el caller llama tft_push_color() y luego CS_HIGH si lo desea */
}

void tft_push_color(uint16_t color) {

    /* Asume que CS ya está LOW y DC HIGH (tras tft_set_addr_window) */
    ssp0_send16(color);
}

void tft_draw_pixel(int16_t x, int16_t y, uint16_t color) {

    if ((x < 0) || (x >= (int16_t)_width) ||
        (y < 0) || (y >= (int16_t)_height)) return;

    tft_set_addr_window((uint16_t)x, (uint16_t)y,
                         (uint16_t)x, (uint16_t)y);
    ssp0_send16(color);
    ssp0_flush();
    TFT_CS_HIGH();
}

void tft_fill_screen(uint16_t color) {

    tft_set_addr_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);
    /* Pre-calcular los dos bytes del color una sola vez. */
    const uint8_t hi = (uint8_t)(color >> 8);
    const uint8_t lo = (uint8_t)(color & 0xFF);

    uint32_t total = (uint32_t)TFT_WIDTH * TFT_HEIGHT; /* 76800 */

    while (total >= 4) {
        /* Esperar TX FIFO Empty (TFE, bit 0): garantiza 8 huecos libres */
        while (!(LPC_SSP0->SR & (1u << 0))) {};

        /* 4 píxeles = 8 bytes, sin esperas intermedias */
        LPC_SSP0->DR = hi; LPC_SSP0->DR = lo;
        LPC_SSP0->DR = hi; LPC_SSP0->DR = lo;
        LPC_SSP0->DR = hi; LPC_SSP0->DR = lo;
        LPC_SSP0->DR = hi; LPC_SSP0->DR = lo;

        total -= 4;
    }

    /* Esperar que el shift register vacíe el último byte antes de CS_HIGH */
    while (LPC_SSP0->SR & (1u << 4)) {}; /* BSY */
    ssp0_flush();
    TFT_CS_HIGH();
}

/* =========================================================
 *  GFX — Líneas
 * ========================================================= */

void gfx_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color) {

    if (w <= 0) return;
    tft_set_addr_window((uint16_t)x, (uint16_t)y,
                         (uint16_t)(x + w - 1), (uint16_t)y);
    for (int16_t i = 0; i < w; i++) ssp0_send16(color);
    ssp0_flush();
    TFT_CS_HIGH();
}

void gfx_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color) {

    if (h <= 0) return;
    tft_set_addr_window((uint16_t)x, (uint16_t)y,
                         (uint16_t)x, (uint16_t)(y + h - 1));
    for (int16_t i = 0; i < h; i++) ssp0_send16(color);
    ssp0_flush();
    TFT_CS_HIGH();
}

/* Bresenham genérico */

void gfx_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {

    /* Optimizaciones para líneas axiales */
    if (x0 == x1) { gfx_draw_vline(x0, (y0 < y1 ? y0 : y1), abs(y1 - y0) + 1, color); return; }
    if (y0 == y1) { gfx_draw_hline((x0 < x1 ? x0 : x1), y0, abs(x1 - x0) + 1, color); return; }

    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)   { int16_t t; t=x0; x0=y0; y0=t; t=x1; x1=y1; y1=t; }
    if (x0 > x1) { int16_t t; t=x0; x0=x1; x1=t; t=y0; y0=y1; y1=t; }

    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++) {
        if (steep) tft_draw_pixel(y0, x0, color);
        else       tft_draw_pixel(x0, y0, color);
        err -= dy;
        if (err < 0) { y0 += ystep; err += dx; }
    }
}

/* =========================================================
 *  GFX — Rectángulos
 * ========================================================= */
uint16_t gfx_get_color(uint32_t pieza) {

    switch (pieza >> 16) {

        case 1:
            return TETRIS_CYAN; // LA FIGURA I
        case 2:
            return TETRIS_AZUL; // LA FIGURA L INVERTIDA
        case 3:
            return TETRIS_NARANJA; // LA FIGURA L
        case 4:
            return TETRIS_AMARILLO; // LA FIGURA CUADRADA
        case 5:
            return TETRIS_VERDE; // LA FIGURA S
        case 6:
            return TETRIS_VIOLETA; // LA FIGURA T
        case 7:
            return TETRIS_ROJO; // ES LA FIGURA Z
        case 8:
            return TETRIS_GRIS;
        case 9:
            return TETRIS_CYAN_SOMBRA;
        case 10:
            return TETRIS_AZUL_SOMBRA;
        case 11:
            return TETRIS_NARANJA_SOMBRA;
        case 12:
            return TETRIS_AMARILLO_SOMBRA;
        case 13:
            return TETRIS_VERDE_SOMBRA;
        case 14:
            return TETRIS_VIOLETA_SOMBRA;
        case 15:
            return TETRIS_ROJO_SOMBRA;
        default:
            return TETRIS_GRIS_SOMBRA;
    }
}

void gfx_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

    gfx_draw_hline(x, y, w, color);
    gfx_draw_hline(x,y+h-1, w, color);
    gfx_draw_vline(x, y, h, color);
    gfx_draw_vline(x+w-1, y, h, color);
}

void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

    /* Enviar bloque de píxeles en una sola ventana — muy eficiente */
    if ((x >= (int16_t)_width)  || (y >= (int16_t)_height)) return;
    if (w <= 0 || h <= 0) return;

    if (x + w > (int16_t)_width)  w = (int16_t)_width  - x;
    if (y + h > (int16_t)_height) h = (int16_t)_height - y;

    tft_set_addr_window((uint16_t)x, (uint16_t)y,(uint16_t)(x+w-1), (uint16_t)(y+h-1));
    uint32_t total = (uint32_t)w * (uint32_t)h;

    while (total >= 16) {
        ssp0_send16(color); ssp0_send16(color); ssp0_send16(color); ssp0_send16(color);
        ssp0_send16(color); ssp0_send16(color); ssp0_send16(color); ssp0_send16(color);
        ssp0_send16(color); ssp0_send16(color); ssp0_send16(color); ssp0_send16(color);
        ssp0_send16(color); ssp0_send16(color); ssp0_send16(color); ssp0_send16(color);

        total -= 16; // Le restamos 16 de golpe a la cuenta
    }

    // 5. Limpieza (por si el tamaño del bloque no era múltiplo exacto de 16)
    while (total--) {
        ssp0_send16(color);
    }
    ssp0_flush();
    TFT_CS_HIGH();
}

void gfx_draw_mini_tetris_block(int16_t x, int16_t y, uint32_t dataPieza) {

    /* Enviar bloque de píxeles en una sola ventana — muy eficiente */
    tft_set_addr_window(x, x,(uint16_t)(x+TAM_BLOQUE_PX-1), (uint16_t)(y+TAM_BLOQUE_PX-1));

    uint16_t color;
    int altura = 0;
    int16_t py = 0;
    int16_t px = 0;

    // ESTE NUMERO ES EL SELECTOR DEL COLOR (LOS ULTIMOS 16 BITS DEL "ARRAY"
    color = gfx_get_color(dataPieza);

    for (int i = 0; i <= 16; i++) {

        int modulo_4 = i%4;
        if (modulo_4 == 0 && i != 0) {
            altura ++;
        }

        if ((dataPieza & (1 << i)) != 0) {
            px = x + (i * TAM_MINI_BLOQUE_PX);
            py = y + (altura * TAM_MINI_BLOQUE_PX);
            gfx_draw_rect(px, py, TAM_MINI_BLOQUE_PX, TAM_MINI_BLOQUE_PX, color);
        }
    }

    ssp0_flush();
    TFT_CS_HIGH();
}

void gfx_draw_tetris_block(int16_t x, int16_t y, uint32_t dataPieza) {

    /* Enviar bloque de píxeles en una sola ventana — muy eficiente */
    tft_set_addr_window(x, y,(uint16_t)(x+TAM_BLOQUE_PX-1), (uint16_t)(y+TAM_BLOQUE_PX-1));

    uint16_t color;
    int altura = 0;
    int16_t py = 0;
    int16_t px = 0;

    // ESTE NUMERO ES EL SELECTOR DEL COLOR (LOS ULTIMOS 16 BITS DEL "ARRAY"
    color = gfx_get_color(dataPieza);

    for (int i = 0; i <= 16; i++) {

        int modulo_4 = i%4;
        if (modulo_4 == 0 && i != 0) {
            altura ++;
        }

        if (dataPieza & (0x8000 >> i)) {
            px = x + (i * TAM_BLOQUE_PX);
            py = y + (altura * TAM_BLOQUE_PX);
            gfx_draw_rect(px, py, TAM_BLOQUE_PX, TAM_BLOQUE_PX, color);
        }
    }

    ssp0_flush();
    TFT_CS_HIGH();
}

void gfx_fill_tetris_block(int16_t x, int16_t y, uint32_t dataPieza) {

    /* Enviar bloque de píxeles en una sola ventana — muy eficiente */
    tft_set_addr_window(x, y,(uint16_t)(x+TAM_BLOQUE_PX-1), (uint16_t)(y+TAM_BLOQUE_PX-1));

    uint16_t color;
    int altura = 0;
    int16_t py = 0;
    int16_t px = 0;

    // ESTE NUMERO ES EL SELECTOR DEL COLOR (LOS ULTIMOS 16 BITS DEL "ARRAY"
    color = gfx_get_color(dataPieza);

    for (int i = 0; i <= 16; i++) {

        int modulo_4 = i%4;
        if (modulo_4 == 0 && i != 0) {
            altura ++;
        }

        if ((dataPieza & (1 << i)) != 0) {
            px = x + (i * TAM_BLOQUE_PX);
            py = y + (altura * TAM_BLOQUE_PX);
            gfx_fill_rect(px, py, TAM_BLOQUE_PX, TAM_BLOQUE_PX, color);
        }
    }

    ssp0_flush();
    TFT_CS_HIGH();
}

/* =========================================================
 *  GFX — Círculos (Bresenham)
 * ========================================================= */

static void _draw_circle_helper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, uint16_t color) {

    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2*r;
    int16_t x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        if (corners & 0x4) { tft_draw_pixel(x0+x, y0+y, color); tft_draw_pixel(x0+y, y0+x, color); }
        if (corners & 0x2) { tft_draw_pixel(x0+x, y0-y, color); tft_draw_pixel(x0+y, y0-x, color); }
        if (corners & 0x8) { tft_draw_pixel(x0-y, y0+x, color); tft_draw_pixel(x0-x, y0+y, color); }
        if (corners & 0x1) { tft_draw_pixel(x0-y, y0-x, color); tft_draw_pixel(x0-x, y0-y, color); }
    }
}

void gfx_draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

    tft_draw_pixel(x0,   y0+r, color);
    tft_draw_pixel(x0,   y0-r, color);
    tft_draw_pixel(x0+r, y0,   color);
    tft_draw_pixel(x0-r, y0,   color);
    _draw_circle_helper(x0, y0, r, 0xF, color);
}

static void _fill_circle_helper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {

    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2*r;
    int16_t x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        if (corners & 0x1) {
            gfx_draw_vline(x0+x, y0-y, 2*y+1+delta, color);
            gfx_draw_vline(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (corners & 0x2) {
            gfx_draw_vline(x0-x, y0-y, 2*y+1+delta, color);
            gfx_draw_vline(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

void gfx_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

    gfx_draw_vline(x0, y0-r, 2*r+1, color);
    _fill_circle_helper(x0, y0, r, 0x3, 0, color);
}

/* =========================================================
 *  GFX — Triángulos
 * ========================================================= */

void gfx_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    gfx_draw_line(x0, y0, x1, y1, color);
    gfx_draw_line(x1, y1, x2, y2, color);
    gfx_draw_line(x2, y2, x0, y0, color);
}

void gfx_fill_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    /* Ordenar vértices por Y */
    int16_t a, b, y, last;
    if (y0 > y1) { int16_t t; t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t; }
    if (y1 > y2) { int16_t t; t=x1;x1=x2;x2=t; t=y1;y1=y2;y2=t; }
    if (y0 > y1) { int16_t t; t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t; }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1; else if (x1 > b) b = x1;
        if (x2 < a) a = x2; else if (x2 > b) b = x2;
        gfx_draw_hline(a, y0, b-a+1, color);
        return;
    }

    int16_t dx01 = x1-x0, dy01 = y1-y0;
    int16_t dx02 = x2-x0, dy02 = y2-y0;
    int16_t dx12 = x2-x1, dy12 = y2-y1;
    int32_t sa = 0, sb = 0;

    last = (y1 == y2) ? y1 : y1 - 1;
    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01; sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        gfx_draw_hline(a, y, b-a+1, color);
    }
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12; sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        gfx_draw_hline(a, y, b-a+1, color);
    }
}

/**
 * @brief Dibuja una estrella rellena de 5 puntas orientada hacia arriba.
 * @param x0 Centro en el eje X
 * @param y0 Centro en el eje Y
 * @param r  Radio exterior (tamaño desde el centro a una punta)
 * @param color Color en formato RGB565
 */

void gfx_fill_star_5pct(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

    // Radio interior aproximado para una estrella armónica (~38% del exterior)
    int16_t r_in = (r * 38) / 100;
    if (r_in < 1) r_in = 1;

    // Coordenadas precalculadas basadas en aproximaciones trigonométricas fijas (escaladas por r y r_in)
    // Puntas exteriores (0 a 4)
    int16_t ex0 = x0;                         int16_t ey0 = y0 - r;
    int16_t ex1 = x0 + (r * 95) / 100;        int16_t ey1 = y0 - (r * 31) / 100;
    int16_t ex2 = x0 + (r * 59) / 100;        int16_t ey2 = y0 + (r * 81) / 100;
    int16_t ex3 = x0 - (r * 59) / 100;        int16_t ey3 = y0 + (r * 81) / 100;
    int16_t ex4 = x0 - (r * 95) / 100;        int16_t ey4 = y0 - (r * 31) / 100;

    // Valles interiores (0 a 4)
    int16_t ix0 = x0 + (r_in * 59) / 100;     int16_t iy0 = y0 - (r_in * 81) / 100;
    int16_t ix1 = x0 + (r_in * 95) / 100;     int16_t iy1 = y0 + (r_in * 31) / 100;
    int16_t ix2 = x0;                         int16_t iy2 = y0 + r_in;
    int16_t ix3 = x0 - (r_in * 95) / 100;     int16_t iy3 = y0 + (r_in * 31) / 100;
    int16_t ix4 = x0 - (r_in * 59) / 100;     int16_t iy4 = y0 - (r_in * 81) / 100;

    // Dibujar los 5 picos exteriores unidos al centro/valles
    gfx_fill_triangle(ex0, ey0, ix0, iy0, ix4, iy4, color); // Punta Superior
    gfx_fill_triangle(ex1, ey1, ix0, iy0, ix1, iy1, color); // Punta Derecha Superior
    gfx_fill_triangle(ex2, ey2, ix1, iy1, ix2, iy2, color); // Punta Derecha Inferior
    gfx_fill_triangle(ex3, ey3, ix2, iy2, ix3, iy3, color); // Punta Izquierda Inferior
    gfx_fill_triangle(ex4, ey4, ix3, iy3, ix4, iy4, color); // Punta Izquierda Superior

    // Rellenar el pentágono interno central dividiéndolo en 3 triángulos
    gfx_fill_triangle(ix0, iy0, ix1, iy1, ix4, iy4, color);
    gfx_fill_triangle(ix1, iy1, ix3, iy3, ix4, iy4, color);
    gfx_fill_triangle(ix1, iy1, ix2, iy2, ix3, iy3, color);
}

/* =========================================================
 *  GFX — Texto
 * ========================================================= */

void gfx_draw_char(int16_t x, int16_t y, char c,uint16_t fg, uint16_t bg, uint8_t size) {

    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = tft_font5x7[(uint8_t)c - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t bits = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            uint16_t color = (bits & 0x01) ? fg : bg;
            if (size == 1) {
                tft_draw_pixel(x + col, y + row, color);
            } else {
                gfx_fill_rect(x + col*size, y + row*size, size, size, color);
            }
            bits >>= 1;
        }
    }
    /* Columna de separación */
    for (uint8_t row = 0; row < 7; row++) {
        if (size == 1) tft_draw_pixel(x + 5, y + row, bg);
        else           gfx_fill_rect(x + 5*size, y + row*size, size, size, bg);
    }
}

void gfx_draw_string(int16_t x, int16_t y, const char *str, uint16_t fg, uint16_t bg, uint8_t size) {

    while (*str) {
        gfx_draw_char(x, y, *str++, fg, bg, size);
        x += 6 * size;  /* 5 px glyph + 1 px gap */
    }
}

void gfx_draw_int(int16_t x, int16_t y, int32_t val, uint16_t fg, uint16_t bg, uint8_t size) {

    char buf[12];
    //snprintf(buf, sizeof(buf), "%ld", (long)val);
    gfx_draw_string(x, y, buf, fg, bg, size);
}
