
/**
 * 	PIN2DMD spi_out
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

#ifndef SPI_OUT_H_
#define SPI_OUT_H_


// pinout for pinball DMD interface

#define pinDisplayEnable GPIO_Pin_3 	// PA3 - DMD pin 1
#define pinRowData  GPIO_Pin_2 		// PA2 - DMD pin 3
#define pinRowClock GPIO_Pin_1 		// PA1 - DMD pin 5
#define pinColLatch GPIO_Pin_0 		// PA0 - DMD pin 7
#define pinDotClock GPIO_Pin_5 		// PA5 - DMD pin 9
#define pinDotData  GPIO_Pin_7 		// PA7 - DMD pin 11


void spi_out_config();
void spi_out();


#endif /* SPI_OUT_H_ */
