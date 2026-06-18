#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

#define R3_Port 0
#define R3_Pin (1 << 8)

#define Tec_Port 0

#define Tec_Row_1 (1 << 0)
#define Tec_Row_2 (1 << 1)
#define Tec_Col_1 (1 << 2)
#define Tec_Col_2 (1 << 3)
#define Tec_Col_3 (1 << 4)
#define Tec_Col_4 (1 << 5)

void confGPIO(void);
void confADC(void);
void confTimer(void);
uint32_t Timer_GetTicks();
int GetJoyStickInput();
int GetTecladoInput();
int Input_GetAction(Bool EsJoyStick);
void Peripheric_Init(void);
