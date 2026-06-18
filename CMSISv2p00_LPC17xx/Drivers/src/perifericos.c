/*
 * TP_FInal.c
 *
 *  Created on: 4 jun. 2026
 *      Author: solgenero
 */
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include "perifericos.h"

void Peripheric_Init(void)
{
	confADC();
	confGPIO();
	confTimer();

}

void confGPIO(void){

	// filas como salidas en alto
	// columnas como entrada con resistencia pull-up

	PINSEL_CFG_T pinconfRows = {0};
	pinconfRows.port = Tec_Port;
	pinconfRows.pin = 0;
	pinconfRows.func = 0;
	pinconfRows.mode = PINSEL_PULLUP;
	pinconfRows.openDrain = DISABLE;

	uint32_t rowpins = Tec_Row_1 | Tec_Row_2;
	PINSEL_ConfigMultiplePins(&pinconfRows, rowpins);

	GPIO_SetDir(Tec_Port, rowpins, GPIO_OUTPUT);
	GPIO_SetPins(Tec_Port, rowpins);



	PINSEL_CFG_T pinconfcols = {};
	pinconfcols.port = Tec_Port;
	pinconfcols.pin = 0;
	pinconfcols.func = 0;
	pinconfcols.mode = 0;
	pinconfcols.openDrain = DISABLE;

	uint32_t colpins = Tec_Col_1 | Tec_Col_2 | Tec_Col_3 | Tec_Col_4;
	PINSEL_ConfigMultiplePins(&pinconfcols, colpins);

	GPIO_SetDir(Tec_Port, colpins, GPIO_INPUT);



	PINSEL_CFG_T R3Conf = {};
	R3Conf.port = R3_Port;
	R3Conf.pin = R3_Pin;
	R3Conf.func = 0;
	R3Conf.mode = 0;
	R3Conf.openDrain = DISABLE;

	PINSEL_ConfigPin(&R3Conf);
	GPIO_SetDir(R3_Port, R3_Pin, GPIO_INPUT);
}

void confADC(void){
	ADC_Init(60);
	ADC_PowerDown();

	ADC_PinConfig(ADC_CHANNEL_0);
	ADC_PinConfig(ADC_CHANNEL_1);
	ADC_BurstEnable();
	ADC_StartCmd(ADC_START_CONTINUOUS);
	ADC_ChannelEnable(ADC_CHANNEL_0);
	ADC_ChannelEnable(ADC_CHANNEL_1);

	ADC_PowerUp();
}

void confTimer(void){

	TIM_TIMERCFG_T conf = {};

	conf.prescaleOpt = TIM_US;
	conf.prescaleValue = 1000;

	TIM_InitTimer(LPC_TIM0, &conf);
	TIM_Enable(LPC_TIM0);
}

uint32_t Timer_GetTicks(){
	return TIM_ReadTimer(LPC_TIM0);
	//return 0;
}

int prevVal = 0;
int GetJoyStickInput(){
	volatile uint16_t Xval = 0;
	volatile uint16_t Yval = 0;
	volatile Bool R3 = FALSE;

	Xval = ADC_ChannelGetData(ADC_CHANNEL_0);
	Yval = ADC_ChannelGetData(ADC_CHANNEL_1);
	R3 = ((GPIO_ReadValue(R3_Port) & R3_Pin) == 0);
	int _intVal = 0;

	if (R3) {return 5;} // SELECT

	if (Xval <= 50){
		_intVal = 3;
	}else if (Xval >= 4050){
		_intVal = 4;
	}
	else if (Yval >= 4050){
		_intVal = 2;
	} else if (Yval <= 50){
		_intVal = 1;
	}

	if (_intVal != prevVal){
		prevVal = _intVal;
		return _intVal;
	}

	prevVal = _intVal;
	return 0;
}

int GetTecladoInput(){

	GPIO_SetPinState(Tec_Port, Tec_Row_2, RESET);
	uint32_t val = GPIO_ReadValue(Tec_Port);
	Bool Select = ( val & Tec_Col_4 ) ==  0;
	if (Select) return 5;
	GPIO_SetPinState(Tec_Port, Tec_Row_2, SET);


	GPIO_SetPinState(Tec_Port, Tec_Row_1, RESET); // carlos pone fila en bajo
	val = GPIO_ReadValue(Tec_Port);
	Bool Left  = ( val & Tec_Col_1) ==  0;
	Bool Right = ( val & Tec_Col_2) ==  0;
	Bool Up    = ( val & Tec_Col_3) ==  0;
	Bool Down  = ( val & Tec_Col_4) ==  0;
	GPIO_SetPinState(Tec_Port, Tec_Row_1, SET);

	if (Up) return 1;
	if (Down) return 2;
	if (Right) return 3;
	if (Left) return 4;
	return 0;
}

int Input_GetAction(Bool EsJoyStick){

	if (EsJoyStick){
		return GetJoyStickInput();
	}

	return GetTecladoInput();
}
