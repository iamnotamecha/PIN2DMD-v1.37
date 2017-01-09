
/**
 * 	PIN2DMD rgb_out.c
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

#ifndef RGB_OUT_H_
#define RGB_OUT_H_

// pinout for color DMD interface

#define pinRGBDisplayEnable GPIO_Pin_6 	// PC6
#define pinRGBRowA  GPIO_Pin_2 		// PE2
#define pinRGBRowB  GPIO_Pin_3 		// PE3
#define pinRGBRowC  GPIO_Pin_4 		// PE4
#define pinRGBRowD  GPIO_Pin_5 		// PE5
#define pinRGBColLatch GPIO_Pin_6 	// PE6
#define pinRGBDotClock GPIO_Pin_6	// PD6
#define dataPinR1  GPIO_Pin_0 		// PD0
#define dataPinG1  GPIO_Pin_1 		// PD1
#define dataPinB1  GPIO_Pin_2 		// PD2
#define dataPinR2  GPIO_Pin_3 		// PD3
#define dataPinG2  GPIO_Pin_4		// PD4
#define dataPinB2  GPIO_Pin_5 		// PD5


void rgb_out_config();
void rgb_out();


#endif /* RGB_OUT_H_ */
