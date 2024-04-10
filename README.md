# SAMD21_1-line_UART
1-line bare metal serial driver on a SAMD21.

## General description
Below is a mini project to show, how to implement a 1-line serial communication interface on a SAMD21 using bare metal programming.

To be fair, initially I was planning to use the SERCOM libraries that are provided within the Arduino environment for my projects (more precisely, the Adafruit training from here: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial ). And then while I was implementing my own serials following the description, I have found two massive bugs:

1) Calling the “Serial.begin” function on a custom serial resets the PMUX of its pins, even if you have assigned them already. I tested it and indeed, the PMUX values for the pins we wish to use with our custom SERCOM will change when we call the “Serial.begin”: PA16 and PA18 will change from MUX 0x2 – peripheral C - to 0x4 – peripheral E. This is very problematic since we don’t want libraries changing our pin definitions unless we specifically tell them to. Of note, this does not apply to the official Serial1 since that one resets to SERCOM using PA10 and PA11 properly.
2) A custom “Serial” will not have the same interrupt behaviour as the official Serial1. What I mean is that Serial1 – once a Tx has finished – will shut off the UART bus instead of keeping it idle when it encounters a “delay”. Any custom serial bus that is defined by hand will keep the bus idle instead.
Now, these are pretty nasty bugs, and I am pretty sure they aren’t the only ones I would need to tackle if I am to work with SAMs. (Of note, I got rid of the second bug just be rearranging my code so maybe I mucked something up. Nevertheless, my conclusion about library reliability stands…)

As such, I have decided to replace the “serial” library from Arduino and write my own instead.

So, what did I want to have in the end? I wanted an init function for the UART, a write function that takes in a hex value and a read function. Also, as a small extra, I wanted to be able to ditch one of the UART lines and make our own 1-line UART coms system by switching the Tx and Rx pins, if possible (it is).

## Previous relevant projects
- SAMD21_ClockDriver

## To read
Getting familiar with the two main chapters of the SAMD21 datasheet on SERCOM and UART are crucial. Also, we need the multiplexing table for the pins:
- I/O Multiplexing and Considerations – pin mux table
- SERCOM Serial Communication Interface – a general description of how the SERCOM works
- SERCOM USART – how to make the SERCOM work as a serial communicator

## Particularities
1) Pin multiplexing
The PMUX registers are one of those that I still can’t quite get my head around. I mean, they are not that difficult to unravel but for such a crucial register one would expect some form of simplicity to avoid any mistakes. No, PMUX values are divided into even and odd groups where if one wants to manipulate an even numbered pin, one has to update PMUXE, while for odd pins, PMUXO. Putting PMUXE values on an odd pin would not work and vica-versa. (Come to think of it, on STs the MODER registers are doing the same thing as the PMUX and they are also the black sheep there. Maybe it is an ARM architecture thing?) 

2) SERCOM
SERCOMs are general communication systems that can be configured to run either I2C, SPI or UART. We have multiple of them (6 on a SAMD21) with some already assigned to one of the above-mentioned interfaces on off the shelf items. SERCOMs are clocked separately through their own generators which must be fed by GCLKs (here I suggest to check the SAMD21_ClockDirver project I did some time ago). SERCOMs interface with pins through “pads” of which all SERCOMs have 4. Exactly where these pads are – on which pins – can be found in the mux table.

3) UART Synch
Like what we have seen with the clock drivers, synchronization is necessary in some cases to ensure functionality. To be precise, whenever we want to change the CTRLB register of the UART, enable the UART or reset the interface, a synchronization step must follow by checking the SYNCBUSY bits.

4) Flipping Rx and Tx
While within STM32 it merely takes one bit flipped (CR2 register’s 15th bit) to swap the Tx and the Rx pins, on the SAMD21 the option is a bit more convoluted. As we discussed above, the Tx and the Rx functions of the SERCOM are attached to pads where Tx can go to pads 0 and 2 and Rx can go to any of the 4. At the same time, one needs to be very careful that not all pads are available on all pins, meaning that picking two pins with available SERCOM pads may only work in one direction but not the other one. A good example is putting SERCOM0 UART on PA10 and PA11 where Tx on PA10 and Rx PA11 would work perfectly, but Tx PA11 would not since that pin has only pad 3 of the SERCOM0 which can not run Tx. Mind, SERCOM0 is the “official” Tx/Rx serial1 on most SAMD21 hobby boards so I guess one wasn’t supposed to muck around with those anyway…

## Explanations
One thing is that I have decided to write all functions as blocking, thus no IRQs are used apart from error detection. This simplified timing and synching the two devices I was hooking up together. Obviously, this is not an efficient solution.
Apart from that, there isn’t much to explain here, the project is rather straightforward.

