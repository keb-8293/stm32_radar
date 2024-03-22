/*
 * uart.h
 *
 *  Created on: Mar 11, 2024
 *      Author: qufql
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

#define STX 0x02
#define ETX 0x03

typedef struct {
	uint8_t command;
	uint16_t data;
} protocol_t;

void initUart(UART_HandleTypeDef *inHuart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
uint16_t getChar();
void transmitPacket(protocol_t data);
protocol_t receivePacket();

#endif /* INC_UART_H_ */
