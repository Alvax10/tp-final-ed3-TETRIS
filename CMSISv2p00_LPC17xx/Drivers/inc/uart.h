/*
 * uart.h
 *
 *  Created on: 12 jun. 2026
 *      Author: solgenero
 */

#ifndef DRIVERS_INC_UART_H_
#define DRIVERS_INC_UART_H_

typedef enum {
    UA_NONE              = 0,
	UA_HANDSHAKE         = 1,  // Master anuncia presencia al Slave
	UA_HANDSHAKE_ACK     = 2,  // Slave confirma conexión
    UA_SEED              = 3,
    UA_SEED_ACK          = 4,
    UA_SEND_ATTACK       = 5,
    UA_RECEIVE_ATTACK    = 6,
    UA_I_LOST            = 7
} UartEvent;

typedef enum {
	UA_ROLE_MASTER       = 0,
	UA_ROLE_SLAVE        = 1,
} UartRole;

#define UART_BAUD (uint32_t) 115200
//Tiempo entre intentos de reconeccion
#define HANDSHAKE_INTERVAL_MS  500U
//Dato para que se entener que estoy por mandar dato
#define UART_START_BYTE 0xAA

//Funciones main
void Uart_Init(int role);
Bool  Uart_IsConnected(void);
int  Uart_CheckNetworkEvents(void);
int  Uart_GetEventValue(void);
void Uart_SendEvent(int eventType, int value);
//Funciones general
void confPines_Tx_Rx();
void confUART();
void DmaEnviar();
void DmaRecibir();


#endif /* DRIVERS_INC_UART_H_ */
