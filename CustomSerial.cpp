/*
 * Created: 10/04/2024 10:12:22
 * Author: BalazsFarkas
 * Project: SAMD21_1-line_UART
 * Processor: SAMD21G18A
 * File: CustomSerial.cpp
 * Program version: 1.0
 */ 

#include "arduino.h"
#include <CustomSerial.h>

//1)SERCOM USART init
void SERCOM1_USART_init(void) {

/*
0)Choose SERCOM
1)Configure IO ports - PORT, PAD
2)Enable clocking - peripheral (GCLK_SERCOM_core), interface (PM)
3)Synch
4)IRQ activation
5)Baud rate
6)Format - S8E or N

*/

//USART init
//1)Configure IO ports
  //Note: PMUX registers are defined differently than others

	PORT->Group[0].PMUX[16>>1].bit.PMUXE = PORT_PMUX_PMUXE_C_Val;		//choose SERCOM on pin
	PORT->Group[0].PMUX[18>>1].bit.PMUXE = PORT_PMUX_PMUXE_C_Val;		//choose SERCOM on pin
	PORT->Group[0].PINCFG[18].bit.INEN = 0; 							//remove input definition - if any
	PORT->Group[0].PINCFG[16].bit.INEN = 0; 							//remove input definition - if any 
	PORT->Group[0].PINCFG[16].bit.DRVSTR = 1;						//stronger current output
	PORT->Group[0].PINCFG[18].bit.DRVSTR = 1;						//stronger current output
	PORT->Group[0].PINCFG[16].bit.PMUXEN = 1;						//enable mux on pin
	PORT->Group[0].PINCFG[18].bit.PMUXEN = 1;						//enable mux on pin


//2)Clock sources
	//We are using DFLL48M on GCLK0

	PM->APBCMASK.bit.SERCOM1_ = 0x1;									//enable SERCOM1 APB


//3)GCLK for SERCOM1	

	GCLK->CLKCTRL.bit.CLKEN = 1,												//clock channel enabled
	GCLK->CLKCTRL.bit.GEN = (0u),												//choose generator, will be generator 0, so GCLK0
	GCLK->CLKCTRL.bit.ID = 0x15;												//choose channel: GCLK_SERCOM1_CORE

	while(GCLK->STATUS.bit.SYNCBUSY);									//we wait until synch is done
	
//4)Set SERCOM1 up
	
	SERCOM1->USART.CTRLA.bit.ENABLE = 0x0;								//disable SERCOM UART
	while(SERCOM1->USART.SYNCBUSY.bit.ENABLE);							//synchronize enable pin

	SERCOM1->USART.CTRLA.bit.MODE = 0x1;								//internal clock for UART
  SERCOM1->USART.CTRLA.bit.SAMPR = 0x1;								//16x oversampling
	SERCOM1->USART.CTRLA.bit.TXPO = 0x1;								//TX on PAD2
	SERCOM1->USART.CTRLA.bit.RXPO = 0x0;								//RX on PAD0
	SERCOM1->USART.CTRLA.bit.FORM = 0x0;								//no parity
//	SERCOM1->USART.CTRLA.bit.FORM = 0x1;								//with parity	
	SERCOM1->USART.CTRLA.bit.DORD = 0x1;								//LSB first

	SERCOM1->USART.CTRLB.bit.TXEN = 0x1;								//enable TX
	SERCOM1->USART.CTRLB.bit.RXEN = 0x1;								//enable RX
	SERCOM1->USART.CTRLB.bit.PMODE = 0x0;								//even parity
	SERCOM1->USART.CTRLB.bit.SBMODE = 0x0;								//1 stop bit
	SERCOM1->USART.CTRLB.bit.CHSIZE = 0x0;								//8 bit word size

	while(SERCOM1->USART.SYNCBUSY.bit.CTRLB);							//synchronize CTRLB register
//	SERCOM1->USART.BAUD.reg = 0x34;										//baud rate for 57600, 48 MHz internal clock source (value extracted from Arduino definition)
	SERCOM1->USART.BAUD.reg = 0x204E;									//baud rate for 38400, 48 MHz internal clock source (value extracted from Arduino definition)

//  SERCOM1->USART.INTENSET.bit.DRE = 0x1;              //data register empty IRQ set - we don't use the interrupt, we use blocking Tx instead
//  SERCOM1->USART.INTENSET.bit.TXC = 0x1;              //Tx complete IRQ set
//  SERCOM1->USART.INTENSET.bit.RXC = 0x1;              //Rx complete IRQ set
  SERCOM1->USART.INTENSET.bit.ERROR = 0x1;            //ERROR - check STATUS register for the flags!

}

//2)USART enable
void SERCOM1_USART_begin(void)
{

	SERCOM1->USART.CTRLA.bit.ENABLE = 0x1;								//enable SERCOM UART
	while(SERCOM1->USART.SYNCBUSY.bit.ENABLE);							//synchronize enable pin

}

//3)USART disable
void SERCOM1_USART_end(void)
{

	SERCOM1->USART.CTRLA.bit.ENABLE = 0x0;								//disable SERCOM UART
	while(SERCOM1->USART.SYNCBUSY.bit.ENABLE);							//synchronize enable pin

}


//4)USART read
uint16_t SERCOM1_USART_read(void) {

  while(!(SERCOM1->USART.INTFLAG.bit.RXC));                 //while the receive complete flag is not set (i.e. while we have not recieved a reply), we block progress of the code
  uint16_t rx_byte_read = SERCOM1->USART.DATA.bit.DATA;
  return rx_byte_read;

}


//5)USART write
void SERCOM1_USART_write(uint8_t tx_byte) {

  SERCOM1->USART.DATA.bit.DATA = tx_byte;                 //we put new data into the USART for TX and remove the DRE flag
  while(!(SERCOM1->USART.INTFLAG.bit.DRE));               //we wait until the data register is not empty - i.e. the data has been transferred to the shift register

}


//6)IRQ handler function
void SERCOM1_Serial_IrqHandler(void)
{

//------ERROR-----///
  //if FERR or PERR is the source of the ERROR
  if (SERCOM1->USART.STATUS.bit.FERR | SERCOM1->USART.STATUS.bit.PERR) {         //if we had FERR or PERR

    uint16_t temp = SERCOM1->USART.DATA.bit.DATA;     //reset RXC
    SERCOM1->USART.STATUS.bit.FERR = 1;               //reset FERR
    SERCOM1->USART.STATUS.bit.PERR = 1;               //reset PERR

  } else {

    //do nothing

  }

  if(SERCOM1->USART.INTFLAG.bit.ERROR) {                                         //if we had an ERROR interrupt - which activates through the STAUTS register flags

    SERCOM1->USART.INTFLAG.bit.ERROR = 1;                                       //we clear the flag
    SERCOM1->USART.STATUS.reg = 0x0;                                            //we clear all the ERROR flags, just in case         

  }
//------ERROR-----///


}

//7)SERCOM handler assignment
void SERCOM1_Handler()
{
	
  void SERCOM1_Serial_IrqHandler(void);
  
}


//-----------------------Programming/coms pin setup-------------------------------//
//8)SERCOM handler assignment
void flip_Rx_Tx() {
  /*
  This function flips the Tx and Rx pins on the pre-set Serial1 USART. This is necessary to have one-line programming between the device and the bootmaster. We call the function every time when we expect an answer from the device.
  Serial1 was chosen as such since on a completely custom serial, the exact parameters of Serial1 could not be reproduced (exact pin muxing, GPIO setup, IRQs and USART specs could not be easily found out)
  The Tx/Rx flip is simply reversed upon re-initializing the serial - i.e. calling Serial1.begin()
  Unlike PA10/11, the PA16/18 solution is fully reversable (though we won't need it)

  To check the original Serial1:
  For the original setup where Tx is on PA18 (PAD[2]) and Rx on PA16 (PAD[0])
  SERCOM1->USART.CTRLA.reg = 0100000000(00)00(01)0010000000000100				0x40012004

  For the upside-down setup where Tx is on PA16 (not used) and Rx on PA18 (PAD[2])
  SERCOM1->USART.CTRLA.reg = 0100000000(10)00(00)0010000000000100				0x40202004

  */

  SERCOM1->USART.CTRLA.bit.ENABLE = 0x0;                  //SERCOM1 disable
  while(SERCOM1->USART.SYNCBUSY.bit.ENABLE);							//synchronize enable pin  
  SERCOM1->USART.CTRLA.reg = 0x40202004;                  //we flip Rx and Tx positions - Rx goes to SERCOM1 PAD[2] instead of Tx. Tx is retargeted to SERCOM1 PAD[0] on PA16.
  SERCOM1->USART.CTRLA.bit.ENABLE = 0x1;                  //SERCOM1 enable
  while(SERCOM1->USART.SYNCBUSY.bit.ENABLE);							//synchronize enable pin

}
//-----------------------Programming/coms pin setup-------------------------------//
