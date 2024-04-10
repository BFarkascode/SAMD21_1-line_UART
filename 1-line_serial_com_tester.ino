/*
 *  Created: 10/04/2024 09:05:18
 *  Author: BalazsFarkas
 *  Project: SAMD21_1-line_UART
 *  Processor: SAMD21G18
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  File: 1-line_serial_com_tester.ino
 *  Hardware description/pin distribution: UART on PA16/PA18
 *  Change history: 
*/

#include <stdlib.h>
#include <CustomSerial.h>

//------------For incoming bytes-----------//

int inc_byte_1;
int inc_byte_2;
int inc_byte_3;
int inc_byte_4;


//------------For incoming bytes-----------//


void setup() {

  // start serial port at 9600 bps:
  Serial.begin(9600);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }

  Serial.println("Starting");
  Serial.print("Created: ");
  Serial.print(__TIME__);
  Serial.print(", ");
  Serial.println(__DATE__);

}

void loop() {


  //------------Custom USART SERCOM1 init-------------//

  SERCOM1_USART_init();
  SERCOM1_USART_begin();

  //------------Custom USART SERCOM1 init-------------//

  //--------Send commands-------//

  Serial.print("Decimal code for the command byte: ");

  while (Serial.available() == 0) {
  }

  uint8_t Command_Byte_Dec = Serial.parseInt();            //here we read in the command from serial monitor

  Serial.println(Command_Byte_Dec);

  switch (Command_Byte_Dec) {                              //We have two commands: one to send over a byte and a second one to receive data from a receiver
    
    case 0:  //Send a command
      Serial.println(" ");
      Serial.println("Sending a command byte over... ");
      Serial.println(" ");

      //The following sequence of bytes are used on the receiver's side to recognise an incoming command
      SERCOM1_USART_write(0xF0);
      SERCOM1_USART_write(0xF0);
      //Send command byte
      SERCOM1_USART_write(0x99);

      //Note: while on official serial1 we would need a delay here, we can omit in our custom solution since the com functions are blocking
      break;

    case 1:  //Data readout
      Serial.println(" ");
      Serial.println("Requesting data bytes from receiver... ");
      SERCOM1_USART_write(0xF0);
      SERCOM1_USART_write(0xF0);
      SERCOM1_USART_write(0x30);  //00110000
      
      //------------Flip Rx/Tx pins------------//
      while(!(SERCOM1->USART.INTFLAG.bit.TXC));             //we wait until all has been sent to the receiver
      flip_Rx_Tx();                                         //then we flip IMMEDIATELLY
      //------------Flip Rx/Tx pins------------//

      inc_byte_1 = SERCOM1_USART_read();
      inc_byte_2 = SERCOM1_USART_read();
      inc_byte_3 = SERCOM1_USART_read();
      inc_byte_4 = SERCOM1_USART_read();

      Serial.print("Incoming byte sequence: ");
      Serial.print(inc_byte_4, HEX);
      Serial.print("-");                        //we print a dash between bytes to not accidentally lose the "0"s in bytes such as "0C"
      Serial.print(inc_byte_3, HEX);
      Serial.print("-"); 
      Serial.print(inc_byte_2, HEX);
      Serial.print("-"); 
      Serial.println(inc_byte_1, HEX);
      break;

    default:
      Serial.println("Error! Command not found");
      break;
  }

  //------------Custom USART SERCOM1 shut off-------------//

  SERCOM1_USART_end();
        
  //------------Custom USART SERCOM1 shut off-------------//

  //--------Send commands-------//
}
