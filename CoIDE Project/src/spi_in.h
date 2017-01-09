/**
 * 	PIN2DMD spi_in.h
 * 	read from spi interface dmd data from real pinball
 *
 *      (c) 2015 by Joerg Amann, Stefan Rinke
 *
 *	This work is licensed under a Creative
 *	Commons Attribution-NonCommercial-
 *	ShareAlike 4.0 International License.
 *
 *	http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 */
#ifndef _SPI_IN_H_
#define _SPI_IN_H_


#define pinDisplayEnable GPIO_Pin_3 	// PA3 - DMD pin 1
#define pinRowData  GPIO_Pin_2 		// PA2 - DMD pin 3
#define pinRowClock GPIO_Pin_1 		// PA1 - DMD pin 5
#define pinColLatch GPIO_Pin_0 		// PA0 - DMD pin 7
#define pinDotClock GPIO_Pin_5 		// PA5 - DMD pin 9
#define pinDotData  GPIO_Pin_7 		// PA7 - DMD pin 11

extern volatile int bufferReceived;

#define frameGroupSize 4

#define DMATransferSize 512*frameGroupSize
#define DMATransferBufferSize (DMATransferSize)+8
extern uint8_t dmarxbuf[DMATransferBufferSize];

void wpc_in_config();
void sam_in_config();
void whitestar_in_config();
void dataeast_in_config();
void gottlieb_in_config();
void wpc95_in_config();

#endif
