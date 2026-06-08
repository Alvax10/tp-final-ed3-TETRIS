/**
 * @file tft_ili9341.h
 * @brief Driver para display TFT ILI9341 (240x320) via SSP0 en LPC1769
 *
 * Arquitectura basada en Adafruit GFX:
 *   - Capa HAL:      ssp0_init(), ssp0_send_byte(), gpio de control
 *   - Capa display:  ili9341_init(), ili9341_draw_pixel(), setAddrWindow()
 *   - Capa GFX:      gfx_draw_line(), gfx_fill_rect(), gfx_draw_circle(),
 *                    gfx_draw_char(), gfx_draw_string()
 *
 * Pines SSP0 LPC1769 (P0.15–P0.18):
 *   SCK  → P0.15
 *   MISO → P0.17  (no usado en escritura)
 *   MOSI → P0.18
 *   CS   → configurable (ver TFT_CS_PORT / TFT_CS_PIN)
 *   DC   → configurable (ver TFT_DC_PORT / TFT_DC_PIN)
 *   RST  → configurable (ver TFT_RST_PORT / TFT_RST_PIN)
 */

#ifndef TFT_ILI9341_H
#define TFT_ILI9341_H

#include <stdint.h>

/* =========================================================
 *  Configuración de pines — ajustar según tu hardware
 * ========================================================= */

/* Chip Select: P0.16 */
#define TFT_CS_PORT     LPC_GPIO0
#define TFT_CS_PIN      16

/* Data/Command (D/C): P2.0 */
#define TFT_DC_PORT     LPC_GPIO2
#define TFT_DC_PIN      0

/* Reset: P2.1 */
#define TFT_RST_PORT    LPC_GPIO2
#define TFT_RST_PIN     1

/* =========================================================
 *  Dimensiones del display
 * ========================================================= */
#define TFT_WIDTH       240
#define TFT_HEIGHT      320

/* =============================================
 *      COLORES NECESARIOS PARA EL JUEGO
 * ============================================= */

// ------------------ COLORES DEL JUEGO ------------------------------
#define TETRIS_AMARILLO TFT_COLOR(200, 200, 200)
#define TETRIS_NARANJA TFT_COLOR(142, 142, 142)
#define TETRIS_VERDE TFT_COLOR(160, 160, 160)
#define TETRIS_GRIS TFT_COLOR(100, 100, 100)
#define TETRIS_CYAN TFT_COLOR(168, 168, 168)
#define TETRIS_VIOLETA TFT_COLOR(95, 95, 95)
#define TFT_WHITE TFT_COLOR(255, 255, 255)
#define TETRIS_AZUL TFT_COLOR(73, 73, 73)
#define TETRIS_ROJO TFT_COLOR(91, 91, 91)
#define TFT_BLACK TFT_COLOR(7, 7, 7)

// ------------------ SOMBRAS DE LOS COLORES PRINCIPALES --------------
#define TETRIS_AMARILLO_SOMBRA TFT_COLOR(130, 130, 130)
#define TETRIS_NARANJA_SOMBRA TFT_COLOR(110, 110, 110)
#define TETRIS_VIOLETA_SOMBRA TFT_COLOR(68, 68, 68)
#define TETRIS_CYAN_SOMBRA TFT_COLOR(94, 94, 94)
#define TETRIS_VERDE_SOMBRA TFT_COLOR(50, 50, 50)
#define TETRIS_AZUL_SOMBRA TFT_COLOR(42, 42, 42)
#define TETRIS_GRIS_SOMBRA TFT_COLOR(44, 44, 44)
#define TETRIS_ROJO_SOMBRA TFT_COLOR(50,50, 50)

// -------------------------- COLORES DE LA UI -------------------------
#define TETRIS_SELECTION_MODE TFT_COLOR(120, 120, 120)
#define TETRIS_WINNER_COLOR TFT_COLOR(102, 102, 102)
#define TETRIS_LOSER_COLOR TETRIS_ROJO
#define TETRIS_STARS TFT_WHITE

/** Convierte componentes R,G,B (0-255) a RGB565 */
#define TFT_COLOR(r, g, b) \
    ((uint16_t)(((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

/** Definiciones necesarias **/
#define TAM_MATRIZ 4
#define TAM_BLOQUE_PX 16
#define TAM_MINI_BLOQUE_PX 6

typedef struct {
 uint8_t tetris_bloques[TAM_MATRIZ][TAM_MATRIZ];
} Bloque_tetris;

/* =========================================================
 *  Orientación (rotación)
 * ========================================================= */
typedef enum {
    TFT_ROT_0   = 0,   /**< Portrait  240×320 */
    TFT_ROT_90  = 1,   /**< Landscape 320×240 */
    TFT_ROT_180 = 2,   /**< Portrait  invertido */
    TFT_ROT_270 = 3,   /**< Landscape invertido */
} tft_rotation_t;

typedef struct {
     uint8_t mapa_matricial[18][11];
} MapaTetris;

/* =========================================================
 *  Fuente de texto integrada (5×7 px, ASCII 32–126)
 * ========================================================= */
extern const uint8_t tft_font5x7[95][5];

/* =========================================================
 *  API — Inicialización
 * ========================================================= */

void delay_ms(uint32_t ms);

/**
 * @brief Inicializa SSP0 y el controlador ILI9341.
 *        Debe llamarse antes que cualquier función*.
 * @param rot  Orientación inicial del display.
 */
void tft_init(tft_rotation_t rot);

/**
 * @brief Cambia la rotación del display en tiempo de ejecución.
 */
void tft_set_rotation(tft_rotation_t rot);

/**
 * @brief Retorna el ancho actual (depende de la rotación).
 */
uint16_t tft_width(void);

/**
 * @brief Retorna el alto actual (depende de la rotación).
 */
uint16_t tft_height(void);

/* =========================================================
 *  API — Primitivas de bajo nivel
 * ========================================================= */

/**
 * @brief Dibuja un único píxel. Es el núcleo del driver.
 */
void tft_draw_pixel(int16_t x, int16_t y, uint16_t color);

/**
 * @brief Rellena todo el display con un color.
 */
void tft_fill_screen(uint16_t color);

/**
 * @brief Define la ventana de escritura activa y activa el envío de datos.
 */
void tft_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Escribe un color RGB565 directamente al bus (tras tft_set_addr_window).
 */
void tft_push_color(uint16_t color);

/* =========================================================
 *  API — Funciones primitivas de alto nivel
 * ========================================================= */

/** @Brief Línea horizontal optimizada (sin trigonometría)
 * int16_t x: Posición en x del display
 * int16_t y: Posición en y del display
 * int16_t w: Ancho de la línea
 * uint16_t color: Color de la línea
 */
void gfx_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color);

/** @Brief Línea vertical optimizada (sin trigonometría)
 * int16_t x: Posición en x del display
 * int16_t y: Posición en y del display
 * int16_t w: Largo de la línea
 * uint16_t color: Color de la línea
 */
void gfx_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color);

/** @Brief Línea arbitraria (algoritmo de Bresenham)
 * int16_t x0: Posición inicial en x del display
 * int16_t x: Posición final en x del display
 * int16_t y0: Posición inicial en y del display
 * int16_t y0: Posición final en y del display
 * uint16_t color: Color de la línea
**/
void gfx_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

/** @Brief Rectángulo vacío
 * int16_t x: Posición en x del display
 * int16_t y: Posición en y del display
 * int16_t w: Ancho del rectángulo
 * int16_t h: Largo del rectángulo
 * uint16_t color: Color del rectángulo
**/
void gfx_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/** @Brief Rectángulo relleno (bloque DMA-friendly)
 * int16_t x: Posición en x del display
 * int16_t y: Posición en y del display
 * int16_t w: Ancho del rectángulo
 * int16_t h: Largo del rectángulo
 * uint16_t color: Color del rectángulo
**/
void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/** @Brief Cualquier bloque de una pieza de tetris miniatura para la UI NEXT PIECE
 * int16_t x: Posición en x donde empieza a dibujarse el bloque
 * int16_t y: Posición en y donde empieza a dibujarse el bloque
 * uint32_t dataPieza: información del color y la pieza como una matriz de bits
 * Example: 0x44444 ( primer 4: color, el resto muestra la pieza I )
**/
void gfx_draw_mini_tetris_block(int16_t x, int16_t y, uint32_t dataPieza);

/** @Brief Cualquier bloque de una pieza de tetris
 * int16_t dataPieza: información del color y la pieza como una matriz de bits
 * int16_t x: Primer pixel de x de la pieza completa en el display
 * int16_t y: Primer pixel de y de la pieza completa en el display
 * Example: 0x44444 ( primer 4: color, el resto muestra la pieza I )
**/
void gfx_draw_tetris_block(int16_t x, int16_t y, uint32_t pieza);

/** @Brief Rellena bloque que pasa de activo a inactivo
 * int16_t dataPieza: información del color y la pieza como una matriz de bits
 * int16_t x: Primer pixel de x de la pieza completa en el display
 * int16_t y: Primer pixel de y de la pieza completa en el display
 * Example: 0x44444 ( primer 4: color, el resto muestra la pieza I )
**/
void gfx_fill_tetris_block(int16_t x, int16_t y, uint32_t dataPieza);

/** @Brief Actualiza todo lo que se necesita actualizar del mapa
 * LINES_BEHAVIOR lines_behavior: Añade o remueve líneas
 * int lines: Cantidad de líneas a remover/agregar
 * Mapa_tetris *mapa_tetris: Matriz completa del mapa del juego
 * Example: 0x44444 ( primer 4: color, el resto muestra la pieza I )
**/
void gfx_render_map(int lines, MapaTetris *mapaTetris);

/** @Brief Círculo vacío (Bresenham)
 * int16_t x0: Primer pixel de x del circulo en el display
 * int16_t y0: Primer pixel de y del circulo en el display
 * int16_t r: Radio del circulo
 * uint16_t color: Color del circulo
**/
void gfx_draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/** @Brief Círculo relleno
 * int16_t x0: Primer pixel de x del circulo en el display
 * int16_t y0: Primer pixel de y del circulo en el display
 * int16_t r: Radio del circulo
 * uint16_t color: Color del circulo
**/
void gfx_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/** @Brief Triángulo vacío
 * int16_t x0: Primer punta pixel de x del triangulo en el display
 * int16_t y0: Primer punta pixel de y del triangulo en el display
 * int16_t x1: Segundo punta pixel de x del triangulo en el display
 * int16_t y1: Segundo punta pixel de y del triangulo en el display
 * int16_t x2: Tercero punta pixel de x del triangulo en el display
 * int16_t y2: Tercer punta pixel de y del triangulo en el display
 * uint16_t color: Color del circulo
**/
void gfx_draw_triangle(int16_t x0, int16_t y0,
                        int16_t x1, int16_t y1,
                        int16_t x2, int16_t y2, uint16_t color);

/** @Brief Triángulo relleno
 * int16_t x0: Primer punta pixel de x del triangulo en el display
 * int16_t y0: Primer punta pixel de y del triangulo en el display
 * int16_t x1: Segundo punta pixel de x del triangulo en el display
 * int16_t y1: Segundo punta pixel de y del triangulo en el display
 * int16_t x2: Tercero punta pixel de x del triangulo en el display
 * int16_t y2: Tercer punta pixel de y del triangulo en el display
 * uint16_t color: Color del circulo
**/
void gfx_fill_triangle(int16_t x0, int16_t y0,
                        int16_t x1, int16_t y1,
                        int16_t x2, int16_t y2, uint16_t color);

/** @Brief Estrella rellena de 5 puntos
 * int16_t x0: Primer punta pixel de x del triangulo en el display
 * int16_t y0: Primer punta pixel de y del triangulo en el display
 * int16_t r: Radio de la punta al centro
 * uint16_t color: Color del circulo
**/
void gfx_fill_star_5pct(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/* =========================================================
 *  API — Texto
 * ========================================================= */

/**
 * @brief Dibuja un carácter ASCII (fuente 5×7, escalable).
 * @param x, y      Esquina superior-izquierda del carácter.
 * @param c         Carácter ASCII (32–126).
 * @param fg        Color del trazo.
 * @param bg        Color de fondo (TFT_BLACK para transparente no disponible en modo simple).
 * @param size      Factor de escala (1 = 5×7 px, 2 = 10×14 px, …).
 */
void gfx_draw_char(int16_t x, int16_t y, char c,
                   uint16_t fg, uint16_t bg, uint8_t size);

/**
 * @brief Dibuja una cadena de texto.
 */
void gfx_draw_string(int16_t x, int16_t y, const char *str,
                     uint16_t fg, uint16_t bg, uint8_t size);

/**
 * @brief Dibuja un número entero con signo.
 */
void gfx_draw_int(int16_t x, int16_t y, int32_t val,
                  uint16_t fg, uint16_t bg, uint8_t size);

#endif /* TFT_ILI9341_H */
