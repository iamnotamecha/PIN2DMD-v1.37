
/**
 * 	PIN2DMD rgb_out
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

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"

#include <stdlib.h>
#include "malloc.h"
#include "string.h"

#include "rgb_out.h"
#include "framebuffer.h"


extern u8 rgbbuf[];
//uint8_t rgbOutputBuf[targetSubframeSize*targetSubframes]; // we need two bytes as we use one byte for clock


#define rowCount 16
#define colCount 128
#define bytesPerRow (colCount / 8)

int rgbperiod[8] = {160,320,640,1280,2560,5120,12240,20480}; // timing for up to 8bit colordepth
int rgbdisplayperiod = 0x2FCF;

int rgbsubframe = 0;
int rgbrow = 0;



// DMA for color dmd output
void DMA2_Stream5_IRQHandler() {
	//Clear interrupt flags
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;
	return;
}

void sleep (u32 count) {
	int i;
	for (i=0;i<count;i++){};
}

void rgb_out(void) {

		if (!(GPIOC->IDR & pinRGBDisplayEnable)) { // is display on ?

			GPIOC->BSRRL = pinRGBDisplayEnable; // turn off display
			TIM2->ARR = rgbdisplayperiod - (rgbperiod[rgbsubframe]);

			if (rgbrow < 15) {
				rgbrow++;
			} else {
				if (rgbsubframe < (targetSubframes - 1)) {
					rgbsubframe++;
				} else {
					rgbsubframe = 0;
					//memcpy(rgbOutputBuf,rgbbuf,targetSubframeSize*targetSubframes);
				}
				rgbrow = 0;
			}

			// load data for next row
			// SR was rgbOutputBuf
			DMA2_Stream5->M0AR = (uint32_t)(rgbbuf + ((rgbrow * 256) + (rgbsubframe * targetSubframeSize)));
			DMA2->LIFCR = 0b111101;
			DMA2_Stream5->CR |= DMA_SxCR_EN;

		} else {

			GPIOE->ODR = (GPIOE->IDR & 0xFFC3) | (rgbrow << 2) | 64; // advance rowcounter & latch
			sleep(1);
			GPIOE->BSRRH = pinRGBColLatch;

			TIM2->ARR = (rgbperiod[rgbsubframe]); // set brightness of subframes
			GPIOC->BSRRH = pinRGBDisplayEnable; // Turn on the display now that the column outputs are latched.

		}

}

void rgb_out_config(){

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// GPIOC Configuration - DMD
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOC, &GPIO_InitStruct);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  	// GPIOD Configuration - DMD
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOD, &GPIO_InitStruct);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  	// GPIOE Configuration - DMD
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOE, &GPIO_InitStruct);

	// Clock Enable (DMA)
	//GPIO_DATA_CLOCKCMD(ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	// DMA init (DMA2, Channel6, Stream5)
	DMA_Cmd(DMA2_Stream5, DISABLE);
	DMA_DeInit(DMA2_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_6;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOD->ODR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 256;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 8bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);

	DMA_FlowControllerConfig(DMA2_Stream5, DMA_FlowCtrl_Memory);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM1, ENABLE);

	//Set and enable interrupt
	TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

	//Set and enable interrupt
	DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}

