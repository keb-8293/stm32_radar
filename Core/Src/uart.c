/*
 * uart.c
 *
 *  Created on: Mar 11, 2024
 *      Author: qufql
 */

#include "uart.h"
#include <stdio.h>

UART_HandleTypeDef *myHuart;

#define rxBufferMax 255

int rxBufferGp; //get pointer (read)
int rxBufferPp; //put pointer (write)
uint8_t rxBuffer[rxBufferMax];
uint8_t rxChar;

// init device
void initUart(UART_HandleTypeDef *inHuart){
	myHuart = inHuart;
	HAL_UART_Receive_IT(myHuart, &rxChar, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	rxBuffer[rxBufferPp++] = rxChar;
	rxBufferPp %= rxBufferMax;
	HAL_UART_Receive_IT(myHuart, &rxChar, 1);
}

uint16_t getChar(){
	int16_t result;
	if(rxBufferGp == rxBufferPp) return -1;
	result = rxBuffer[rxBufferGp++];
	rxBufferGp %= rxBufferMax;
	return result;
}

int _write(int file, char *p, int len){
	HAL_UART_Transmit(myHuart, p, len, 10);
	return len;
}

//packet transmit
void transmitPacket(protocol_t data) {
	//ready
	uint8_t txBuffer[] = {STX, 0, 0, 0, 0, ETX};
	txBuffer[1] = data.command;
	txBuffer[2] = (data.data >> 7) | 0x00;
	txBuffer[3] = (data.data & 0x7f) | 0x00;
	//CRC calculate
	txBuffer[4] = txBuffer[0] + txBuffer[1] + txBuffer[2] + txBuffer[3];
	//send data
	HAL_UART_Transmit(myHuart, txBuffer, sizeof(txBuffer), 1);
	//ready to finish data sending
	while(HAL_UART_GetState(myHuart) == HAL_UART_STATE_BUSY_TX
			||HAL_UART_GetState(myHuart) == HAL_UART_STATE_BUSY_TX_RX);
}

//packet receive
protocol_t receivePacket(){
	protocol_t result;
	uint8_t buffer[6];
	uint8_t count = 0;
	uint32_t timeout;

	int16_t ch = getChar();
	memset(&result, 0, sizeof(buffer));
	if(ch == STX){
		buffer[count++] = ch;
		timeout = HAL_GetTick(); //timeout start
		while(ch != ETX){
			ch = getChar();
			if(ch != -1){
				buffer[count++] = ch;
			}
			//timeout calculate
			if(HAL_GetTick() - timeout >= 2) return result;
		}
		//CRC check
		uint8_t crc = 0;
		for(int i = 0; i < 4; i++)
			crc += buffer[i];
		if(crc != buffer[4]) return result;
		//parsing after receive
		result.command = buffer[1];
		result.data = buffer[3] & 0x7f;
		result.data |= (buffer[2] & 0x7f) << 7;
	}
	return result;
}
