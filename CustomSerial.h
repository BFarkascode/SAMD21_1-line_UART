/*
 * Created: 10/04/2024 10:12:22
 * Author: BalazsFarkas
 * Project: SAMD21_1-line_UART
 * Processor: SAMD21G18A
 * File: CustomSerial.h
 * Header version: 1.0
 */ 

#ifndef BFSerial_H_
#define BFSerial_H_

void SERCOM1_USART_init(void);
void SERCOM1_USART_begin(void);
void SERCOM1_USART_end(void);
uint16_t SERCOM1_USART_read(void);
void SERCOM1_USART_write(uint8_t);
void SERCOM1_Serial_IrqHandler(void);
void flip_Rx_Tx(void);

#endif /*BFSerial*/
