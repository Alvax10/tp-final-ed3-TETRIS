/*
 * uart.c
 *
 *  Created on: 11 jun. 2026
 *      Author: solgenero
 */
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "perifericos.h"
#include "stdbool.h"
#include "uart.h"

// Flags de control del DMA
static volatile Bool tx_completed = true;
static volatile Bool rx_buffer_ready = false;

//Flas de control general
static int myRole = 0;
static Bool connected = 0;
static uint16_t value_now = 0;
static uint32_t lastHandshakeMs = 0U;
static UartEvent event_now = 0;

// Buffers para enviar-recibir de 3bytes
static volatile uint8_t bufferTX[3];
static volatile uint8_t bufferRX[3];

// ------ CODGIO MAIN------
void Uart_Init(int role){ //MAIN
	myRole = role;
	connected = false;
	value_now = 0;
	event_now = UA_NONE;
	lastHandshakeMs = 0; //Ultima conexion
	tx_completed = true;
	rx_buffer_ready = false;

	//1- Configuaracion de pines Tx-P0.2 y Rx-P0.3
	confPines_Tx_Rx();
	//2- Inicializacion de UART0
	confUART();
	//3- Inicializacion de DMA
	GPDMA_Init();
	//4- Lanzo una una recepsion continua por DMA

	//5- Habilito las interrupciones
	NVIC_EnableIRQ(UART0_IRQn);

	// Si somos Master, enviamos el primer HANDSHAKE de inmediato. El Slave solo escucha y responde cuando lo recibe.
	if(role == UA_ROLE_MASTER){
		/*Estoy en la placa que apreto el multijugador
		 * Le aviso que que soy MASTER*/
		Uart_SendEvent(UA_HANDSHAKE, UA_ROLE_MASTER);
		lastHandshakeMs = Timer_GetTicks();
	}

}

Bool Uart_IsConnected(){ //MAIN
	event_now = Uart_CheckNetworkEvents();
	value_now = Uart_GetEventValue();
	if(event_now == UA_HANDSHAKE_ACK){
		if(value_now == UA_ROLE_SLAVE){
			myRole = UA_ROLE_SLAVE;
			connected = true;
		}
		return true;
	} else if(!connected) {
		//Si no esta conectado
		//guardar
		if(myRole == UA_ROLE_MASTER && (!connected)){
			//Si somo el master y no esta conectado, reintentamos la conexion
			uint32_t now = Timer_GetTicks();
			if((now - lastHandshakeMs) >= HANDSHAKE_INTERVAL_MS){
				lastHandshakeMs = now;
				Uart_SendEvent(UA_HANDSHAKE, UA_ROLE_MASTER);
			}
		}
	}
	return connected;
}

void Uart_SendEvent(int eventType, int value){ //MAIN
	//Hago un while para saber/esperar a que el DMA termine de ejecutarse, uso la flag del tx
	while(!tx_completed);
	tx_completed = false; //Vuelvo a reiniciar la flag

	//Guardo en los buffer
	bufferTX[0] = UART_START_BYTE;
	bufferTX[1] = (uint8_t)eventType; //EVENTO
	bufferTX[2] = (uint8_t)value; //VALOR

	//Limpio la interrupcion del DMA-TC:
	GPDMA_ClearIntPending(GPDMA_CLR_INTTC, GPDMA_CH_0);

	//Configuracion del DMA para Enviar y que envie
	DmaEnviar();
}

int Uart_CheckNetworkEvents(void){ //MAIN
	//1- Si el DMA no detecta un packete entocnes no tengo ningun evento
	if(!rx_buffer_ready){
		return UA_NONE;
	}
	rx_buffer_ready = false;

	//Por las duadas limpiamos la flag del RX
	GPDMA_ClearIntPending(GPDMA_CLR_INTTC, GPDMA_CH_1);

	//2- Ahora tengo que ver/validar lo que me llega en el bufferRX
	if(bufferRX[0] == UART_START_BYTE){
		//Me guardo los datos en variables
		uint8_t event_incoming = bufferRX[1];
		uint8_t value_incoming = bufferRX[2];
		if(event_incoming == UA_HANDSHAKE && myRole == UA_ROLE_SLAVE ){
			/*Basicamente a la otra LPC le llego que la "llamaron"
			 * Esta conectado, y tenog que mandarle a la otra placa que esta todo en orden.
			*/
			connected = true;
			Uart_SendEvent(UA_HANDSHAKE_ACK , UA_ROLE_SLAVE);
			DmaRecibir();
			return UA_NONE;
		}
		if(event_incoming == UA_HANDSHAKE_ACK && myRole == UA_ROLE_MASTER){
			/*
			 * La otra placa me dio la confirmacion y soy el master
			 * Activo la flag de coneccion
			 * Llamo al dma que recibe
			 */
			connected = true;
			DmaRecibir();
			return UA_NONE;
		}
		//Guardamos el evento y el valor para el main
		event_now = event_incoming;
		value_now = value_incoming;
		//Llamo al dma que recibe
		DmaRecibir();
		return event_now;
	}else{
	//Llamo al Dma y retorno nada
		DmaRecibir();
		return UA_NONE;
	}
}

int Uart_GetEventValue(void){ //MAIN
	return (uint8_t)value_now;
}

// ------ CODIGO APARTE -------
void confUART(){

	//COnfiguracion de inicializacion de UART
	UART_CFG_T cfgUart = {0};
	cfgUart.baudRate = (uint32_t)UART_BAUD;
	cfgUart.dataBits = UART_DBITS_8;
	cfgUart.parity = UART_PARITY_NONE;
	cfgUart.stopBits = UART_STOPBIT_1;
	UART_Init(UART0, &cfgUart);

	//COnfiguracion de inicializacion de la FIFO
	UART_FIFO_CFG_T cfgFIFO = {0};
	cfgFIFO.dmaMode = ENABLE;
	cfgFIFO.level = UART_FIFO_TRGLEV1; // UART FIFO trigger level 1: 4 character
	cfgFIFO.resetRxBuf = ENABLE;
	cfgFIFO.resetTxBuf = ENABLE;
	UART_FIFOConfig(UART0 , &cfgFIFO);

	//Habilita fuentes de interrupción UART
	//UART_IntConfig(&LPC_UART0, UART_INT_RBR, ENABLE);

	//Habilito el trasmisor Tx:
	UART_TxEnable(UART0);


}

void confPines_Tx_Rx(){
	//COnfiguracion de los pines Tx-P0.2 y Rx-P0.3
	PINSEL_CFG_T cfgPinTX = {0};
	PINSEL_CFG_T cfgPinRX = {0};

	cfgPinTX.port = PORT_0;
	cfgPinTX.pin = PIN_2;
	cfgPinTX.func = PINSEL_FUNC_01;
	cfgPinTX.mode = PINSEL_PULLUP;
	cfgPinTX.openDrain = DISABLE;
	PINSEL_ConfigPin(&cfgPinTX);

	cfgPinRX.port = PORT_0;
	cfgPinRX.pin = PIN_3;
	cfgPinRX.func = PINSEL_FUNC_01;
	cfgPinRX.mode = PINSEL_PULLUP;
	cfgPinRX.openDrain = DISABLE;
	PINSEL_ConfigPin(&cfgPinRX);
}

void DmaEnviar(){

	GPDMA_Channel_CFG_T cfgDMA = {0};
	cfgDMA.channelNum = GPDMA_CH_0;
	cfgDMA.transferSize = 3;
	cfgDMA.type = GPDMA_M2P;
	cfgDMA.srcMemAddr = (uint32_t) bufferTX;
	cfgDMA.dstMemAddr = 0;
	cfgDMA.srcConn = 0;
	cfgDMA.dstConn = GPDMA_UART0_Tx;
	cfgDMA.src.width = GPDMA_BYTE;
	cfgDMA.src.burst = GPDMA_BURST_AUTO;
	cfgDMA.src.increment = ENABLE;
	cfgDMA.dst.width = GPDMA_BYTE;
	cfgDMA.dst.burst = GPDMA_BURST_AUTO;
	cfgDMA.dst.increment = DISABLE;
	cfgDMA.intTC = ENABLE;
	cfgDMA.intErr = DISABLE;
	cfgDMA.linkedList = 0;
	GPDMA_SetupChannel(&cfgDMA);
	GPDMA_ChannelStart(GPDMA_CH_0);

}

void DmaRecibir(){
	GPDMA_Channel_CFG_T cfgDMA = {0};
	cfgDMA.channelNum = GPDMA_CH_1;
	cfgDMA.transferSize = 3;
	cfgDMA.type = GPDMA_P2M;
	cfgDMA.srcMemAddr = 0;
	cfgDMA.dstMemAddr = (uint32_t) bufferRX;;
	cfgDMA.srcConn = GPDMA_UART0_Rx;
	cfgDMA.dstConn = 0;
	cfgDMA.src.width = GPDMA_BYTE;
	cfgDMA.src.burst = GPDMA_BURST_AUTO;
	cfgDMA.src.increment = DISABLE;
	cfgDMA.dst.width = GPDMA_BYTE;
	cfgDMA.dst.burst = GPDMA_BURST_AUTO;
	cfgDMA.dst.increment = ENABLE;
	cfgDMA.intTC = ENABLE;
	cfgDMA.intErr = DISABLE;
	cfgDMA.linkedList = 0;
	GPDMA_SetupChannel(&cfgDMA);
	GPDMA_ChannelStart(GPDMA_CH_1);
}

void DMA_IRQHandler(){
	//Prregunto si fue del canal 0
	if(GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_0) == SET){
		//Si la interrupcion es por el canal 0, limpio la flag
		GPDMA_ClearIntPending(GPDMA_INTTC, GPDMA_CH_0);
		//Ponemos como true la flag de que se completo la transferencia de TX
		tx_completed = true;
	}
	if(GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_1) == SET){
		//Si la interrupcion es por el canal 1, limpio la flag
		GPDMA_ClearIntPending(GPDMA_INTTC, GPDMA_CH_1);
		rx_buffer_ready = true;
	}

}

