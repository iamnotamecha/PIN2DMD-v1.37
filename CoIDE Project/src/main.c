
/**
 * 	PIN2DMD main.
 *
 *      (c) 2016 by Joerg Amann, Stefan Rinke
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
#include "stm32f4xx_dma.h"

#include "stm32_ub_fatfs.h"
#include "stm32_winusb.h"
#include "stm32_ub_systick.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>


#include "framebuffer.h"
#include "matrix.h"
#include "rgb_out.h"
#include "spi_out.h"
#include "spi_in.h"
#include "display.h"

typedef enum { false, true } bool;

#define VERSION "V1.37"

int defaultPalette = 0;
int activePalette = 0;
int nextPalette = 0;

int frameCount = 0;

#define UID_ADDRESS		0x1FFF7A10
#define GetUID(x)		((x >= 0 && x < 12) ? (*(uint8_t *) (UID_ADDRESS + (x))) : 0)
char UID[32];

extern int rgbperiod[];
extern int rgbdisplayperiod;

typedef enum { PinMame_RGB, PinMame_Mono, WPC, Stern, Gottlieb, DataEast, WhiteStar, WPC95 } DeviceMode;

DeviceMode devicemode = 0;
uint8_t palettemode = 1;

uint8_t rxbuf[sizeof(rgbbuf)+4] = {};
uint16_t rxbuf_ptr = 0;

bool newpacket = false;

//TODO almost the same constants are in framebuffer.h USE ONE SET AT A central place
#define ROWS 32
#define COLS 128

#define NUMBER_OF_SUBFRAMES 4
#define BYTES_PER_ROW (COLS/8)
#define SUBFRAMESIZE (BYTES_PER_ROW*ROWS)
#define BUFFER_OFFSET 1

#define BLUE GPIO_Pin_15
#define GREEN GPIO_Pin_12
#define RED GPIO_Pin_14
#define ORANGE GPIO_Pin_13

#define LedToggle(pin) GPIOD->ODR ^= pin
#define LedOn(pin) GPIOD->BSRRL = pin
#define LedOff(pin) GPIOD->BSRRH = pin

void LedSet(pin, val) {
	if(val) LedOn(pin); else LedOff(pin);
}

void GPIO_LED_Button_Init(){

	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  	// GPIOD Configuration - LED
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOD, &GPIO_InitStruct);

	// GPIO init for blue button
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOA, &GPIO_InitStruct);

}

uint16_t readShort(uint8_t* p) {
	return (*p << 8) + *(p+1);
}

uint32_t readInt(uint8_t* p) {
	return (*p << 24) + (*(p+1) << 16) + (*(p+2) << 8) + *(p+3);
}


bool initCardReaderAndMount() {
	SPI_Cmd(SPI2, ENABLE);			// turn on SD Card SPI2 and TIM
	TIM_Cmd(SD_1MS_TIM, ENABLE);
	if(UB_Fatfs_CheckMedia(MMC_0)==FATFS_OK) {
		//  mount media
		if(UB_Fatfs_Mount(MMC_0)==FATFS_OK) {
			return true;
		}
	}
	LedOn(RED);
	return false;
}

void unmountAndDisableCardReader() {
  	UB_Fatfs_UnMount(MMC_0);
	SPI_Cmd(SPI2, DISABLE);			// shut off SD Card SPI2 and TIM
	TIM_Cmd(SD_1MS_TIM, DISABLE);
}


void write_config(void) {

	FIL myFile;
	uint32_t save_cnt = 0;

	rxbuf[0] = devicemode;
	rxbuf[1] = defaultPalette;

    //write rgb display timings

    rxbuf[10]=rgbdisplayperiod>>8;
    rxbuf[11]=rgbdisplayperiod;
    rxbuf[12] = rgbperiod[0]>>8;
    rxbuf[13] = rgbperiod[0];
    rxbuf[14] = rgbperiod[1]>>8;
    rxbuf[15] = rgbperiod[1];
    rxbuf[16] = rgbperiod[2]>>8;
    rxbuf[17] = rgbperiod[2];
    rxbuf[18] = rgbperiod[3]>>8;
    rxbuf[19] = rgbperiod[3];

    if( initCardReaderAndMount() ) {
	  	// create file
	  	if(UB_Fatfs_OpenFile(&myFile, "0:/pin2dmd.dat", F_WR_CLEAR)==FATFS_OK) {
	  		// write content
	  		UB_Fatfs_WriteBlock(&myFile,rxbuf,20,&save_cnt);
	  		LedOff(RED);
	  		// close file
	  		UB_Fatfs_CloseFile(&myFile);
	  	}
    }
  	unmountAndDisableCardReader();
}

void write_displaybuffer(void) {

	FIL myFile;
	uint32_t save_cnt = 0;

	if( initCardReaderAndMount()) {
		// create file
		if (UB_Fatfs_OpenFile(&myFile, "0:/logo.dat", F_WR_CLEAR)
				== FATFS_OK) {
			// write content
			int i;
			for (i = 0; i < 32; i++) {
				UB_Fatfs_WriteBlock(&myFile, rgbbuf + (i * 512), 512,
						&save_cnt);
			}
			LedOff(RED);
			// close file
			UB_Fatfs_CloseFile(&myFile);
		}
	}
	unmountAndDisableCardReader();
}



void read_config(void) {

	FIL myFile;
	uint32_t read_cnt = 0;

	if( initCardReaderAndMount() ) {
		// create file
		if (UB_Fatfs_OpenFile(&myFile, "0:/pin2dmd.dat", F_RD)
				== FATFS_OK) {
			// read content
			UB_Fatfs_ReadBlock(&myFile, &rxbuf[0], 256, &read_cnt);
			// close file
			UB_Fatfs_CloseFile(&myFile);
			devicemode = rxbuf[0];
			defaultPalette = activePalette = nextPalette = rxbuf[1];
			// read rgb display timings
			if (read_cnt > 10 && readShort(rxbuf + 10) != 0) {
				rgbdisplayperiod = readShort(rxbuf + 10);
				rgbperiod[0] = readShort(rxbuf + 12);
				rgbperiod[1] = readShort(rxbuf + 14);
				rgbperiod[2] = readShort(rxbuf + 16);
				rgbperiod[3] = readShort(rxbuf + 18);
			}
		}

	}
	unmountAndDisableCardReader();
}

void load_palette(uint8_t buf[5 + (numberOfColorsPerPalette * 3)]) {
	uint16_t index;
	// UNUSED uint16_t no_of_colors;
	index = readShort(buf);
	// no_of_colors = (buf[2]*256)+buf[3];
	if (buf[4] > 0) { // TODO check logic here
		defaultPalette = activePalette = nextPalette = index;
		palettemode = buf[4];
	}
	addPalette(&(buf[5]), index);
}

void config_mode(int val) {

	switch (val) {

	case 0x00:
		NVIC_SystemReset();
		break;

	case 0x01: //save config to sd card

		write_config();
		newpacket = false;
		break;

	case 0x02: //switch devicemode

		devicemode = rxbuf[5];
		if (devicemode == PinMame_Mono) {
			spi_out_config();
		} else {
			SPI_Cmd(SPI1, DISABLE); // shut off SPI1 in RGB mode
			rgb_out_config();
		}

		// turn off displays during display switch
		GPIOA->BSRRH = pinDisplayEnable;
		GPIOC->BSRRL = pinRGBDisplayEnable;
		newpacket = false;
		break;

	case 0x03: //switch palette

		activePalette = defaultPalette = nextPalette = rxbuf[5];
		createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette );
		newpacket = false;
		break;

	case 0x04: //upload palette
		rxbuf[2] = 0;
		rxbuf[3] = rxbuf[5];
		rxbuf[4] = 0;
		rxbuf[5] = numberOfColorsPerPalette;
		load_palette(&rxbuf[2]);
		createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
		newpacket = false;
		break;

	case 0x05: //upload frame data
		createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
		newpacket = false;
		break;

	case 0x06:
		newpacket = false;
		break;

	case 0x07: //set all settings to default
			deleteAllPalettes();
			initPalettes();
			defaultPalette = activePalette = nextPalette = 0;
			palettemode = 1;
			read_config();
			createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
			newpacket = false;
			break;

	case 0x08: //adjust display timings
			rgbdisplayperiod = readShort(rxbuf+5);
			for(int i = 0; i<=7; i++)
				rgbperiod[i] = readShort(rxbuf+7+i*2);

			createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
			newpacket = false;
			break;

	case 0xFF: //display UID
			promptUID();
			createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
			WINUSB_SendData(UID,15);
			newpacket = false;
			break;

	}

}


void PinMameRGBLoop() {


	while(true) {
		  if(WINUSB_GetStatus()==USB_WINUSB_CONNECTED) {
			  if (newpacket == true) {
				  switch (rxbuf[3]) {
		  	      case 0:
		  	    	newpacket = false;
		  			LedToggle(BLUE);
		  			memcpy(displaybuf,rxbuf+4,displaybufSize);

		  			LedSet(GREEN, activePalette != defaultPalette);

	  				createRGBDisplayBuffer_4bit(displaybuf,rgbbuf,activePalette);
		  			break;

		  	      case 0xFF:
		  	    	config_mode(rxbuf[4]);
		  	    	break;
				  }
			  }
		  }
	  }

}

void PinMameMonoLoop() {


	spi_out_config();

	while(true) {
		if (WINUSB_GetStatus() == USB_WINUSB_CONNECTED) {
			if (newpacket == true) {
				switch (rxbuf[3]) {
				case 0:
					newpacket = false;
					LedToggle(BLUE);
					memcpy(displaybuf, rxbuf + 4, displaybufSize);
					createRGBDisplayBuffer_4bit(displaybuf, rgbbuf, activePalette);
					break;

				case 0xFF:
					config_mode(rxbuf[4]);
					break;
				}
			}
		}
	}
}

void WPCLoop() {

	WPC_in_config();

	while(true) {
		if (WINUSB_GetStatus() == USB_WINUSB_CONNECTED) {
			if (newpacket == true) {
				switch (rxbuf[3]) {
					case 0:
					break;

					case 0xFF:
					config_mode(rxbuf[4]);
					break;
					}
				}
		}
		if ( bufferReceived ) {

			LedToggle(BLUE);

			memset(displaybuf, 0, displaybufSize); // clear displaybuffer
			memcpy(displaybuf, dmarxbuf + 1, subframeSizeInByte*3); // copy data to prevent overwriting through interrupt

			bufferReceived = 0;

			LedOff(ORANGE);

  			LedSet(GREEN, activePalette != defaultPalette);

			createRGBDisplayBuffer_2bit(displaybuf, rgbbuf, activePalette);
		}
	}
}

void Stern4bitLoop() {
	Stern_4bit_in_config();

	while(true) {
		if (WINUSB_GetStatus() == USB_WINUSB_CONNECTED) {
			if (newpacket == true) {
				switch (rxbuf[3]) {
					case 0:
					break;

					case 0xFF:
					config_mode(rxbuf[4]);
					break;
					}
				}
		}
		if( bufferReceived) {
			LedToggle(BLUE);
			memset(displaybuf,0,displaybufSize); // clear displaybuffer

			for(int k=0; k < ROWS; k++ ) {
				memcpy( displaybuf+k*BYTES_PER_ROW,
						dmarxbuf + BYTES_PER_ROW*2 + BUFFER_OFFSET + k * BYTES_PER_ROW*NUMBER_OF_SUBFRAMES, BYTES_PER_ROW );
				memcpy( displaybuf+k*BYTES_PER_ROW + SUBFRAMESIZE*1,
						dmarxbuf + BYTES_PER_ROW*0 + BUFFER_OFFSET + k * BYTES_PER_ROW*NUMBER_OF_SUBFRAMES, BYTES_PER_ROW );
				memcpy( displaybuf+k*BYTES_PER_ROW + SUBFRAMESIZE*2,
						dmarxbuf + BYTES_PER_ROW*1 + BUFFER_OFFSET + k * BYTES_PER_ROW*NUMBER_OF_SUBFRAMES, BYTES_PER_ROW );
				memcpy( displaybuf+k*BYTES_PER_ROW + SUBFRAMESIZE*3,
						dmarxbuf + BYTES_PER_ROW*3 + BUFFER_OFFSET + k * BYTES_PER_ROW*NUMBER_OF_SUBFRAMES, BYTES_PER_ROW );
			}

			bufferReceived = 0;

  			LedSet(GREEN, activePalette != defaultPalette);

			createRGBDisplayBuffer_4bit(displaybuf,rgbbuf,activePalette);
		}
	}
}


void Stern2bitLoop() {

	Stern_2bit_in_config();

	while (1) {
		if (WINUSB_GetStatus() == USB_WINUSB_CONNECTED) {
			if (newpacket == true) {
				switch (rxbuf[3]) {
					case 0:
					break;

					case 0xFF:
					config_mode(rxbuf[4]);
					break;
					}
				}
		}

		if(bufferReceived) {
			LedToggle(BLUE);
			memset(displaybuf,0,displaybufSize); // clear displaybuffer

			for(int k=0; k < ROWS; k++ ) {
				memcpy( displaybuf+k*BYTES_PER_ROW,
						dmarxbuf + BYTES_PER_ROW*0 + BUFFER_OFFSET + k * BYTES_PER_ROW*2, BYTES_PER_ROW );
				memcpy( displaybuf+k*BYTES_PER_ROW + SUBFRAMESIZE*1,
						dmarxbuf + BYTES_PER_ROW*0 + BUFFER_OFFSET + k * BYTES_PER_ROW*2, BYTES_PER_ROW );
				memcpy( displaybuf+k*BYTES_PER_ROW + SUBFRAMESIZE*2,
						dmarxbuf + BYTES_PER_ROW*1 + BUFFER_OFFSET + k * BYTES_PER_ROW*2, BYTES_PER_ROW );
			}

			bufferReceived = 0;

			LedOff(ORANGE);

  			LedSet(GREEN, activePalette != defaultPalette);

			createRGBDisplayBuffer_2bit(displaybuf, rgbbuf, activePalette);
		}
	}
}

void GottliebLoop() {

	Gottlieb_in_config();

	while (1) {
		if (WINUSB_GetStatus() == USB_WINUSB_CONNECTED) {
			if (newpacket == true) {
				switch (rxbuf[3]) {
					case 0:
					break;

					case 0xFF:
					config_mode(rxbuf[4]);
					break;
					}
				}
		}

		if(bufferReceived) {
			LedToggle(BLUE);
			memset(displaybuf, 0, displaybufSize); // clear displaybuffer

			memcpy(displaybuf, dmarxbuf + 1, subframeSizeInByte);
			memcpy(displaybuf+subframeSizeInByte, dmarxbuf + 1 + subframeSizeInByte + 32, subframeSizeInByte);
			memcpy(displaybuf+subframeSizeInByte, dmarxbuf + 1 + subframeSizeInByte*2 + 64, subframeSizeInByte);

			bufferReceived = 0;

			LedOff(ORANGE);

  			LedSet(GREEN, activePalette != defaultPalette);

			createRGBDisplayBuffer_2bit(displaybuf, rgbbuf, activePalette);
		}
	}
}




void promptMode( const char* mode, int x) {
	writeText("                ",0,24,16,0);
	writeText(mode,x,24,strlen(mode),2);
	createRGBDisplayBuffer_4bit(displaybuf,rgbbuf,activePalette);
	UB_Systick_Timer1(TIMER_START_ms,1500);
	while(UB_Systick_Timer1(TIMER_CHECK,0)!=TIMER_HOLD);
}

void promptUID(){
	sprintf(UID, "#%02X%02X%02X%02X%02X%02X%c%c",
			GetUID(0), /* LSB */
			GetUID(1),
			GetUID(2),
			GetUID(3),
			GetUID(4),
			GetUID(5),
			GetUID(6),
			GetUID(7)  /* MSB */
		);
	writeText(UID,5,24,24,2);
}

int main(){

	SystemInit();

	initPalettes();

    UB_Systick_Init(); //in ms

	GPIO_LED_Button_Init();

	UB_Fatfs_Init();
	read_config();

	TIM2_config();

	// check blue button for devicemode 0 (USB active)
	int i;
	int k = 0;
	for (i=0;i<100;i++){if (GPIOA->IDR & 0x0001) {k++;}}
	if (k > 50) {
		LedOn(RED);
		devicemode = PinMame_RGB;
	}

	GPIOC->BSRRL = pinRGBDisplayEnable;  // turn off RGB display
    GPIOA->BSRRH = GPIO_Pin_3; // turn off MONO display

    UB_Systick_Init(); //in ms
	rgb_out_config();

	writeText("OPENSOURCE",4,24,10,10);
    writeText(VERSION,87,24,strlen(VERSION),12);

    createRGBDisplayBuffer_4bit(displaybuf,rgbbuf,activePalette);

	UB_Systick_Timer1(TIMER_START_ms,1500);
	while(UB_Systick_Timer1(TIMER_CHECK,0)!=TIMER_HOLD);

	initCardReaderAndMount();
	WINUSB_Init();

    switch (devicemode) {

    	case PinMame_RGB:	// PinMAME input - RGB output
			promptMode("VIRTUAL PINBALL",5);
        	PinMameRGBLoop();
        	break;

        case PinMame_Mono:	// PinMAME input - monochome DMD output
        	promptMode("CLASSIC",36);
        	PinMameMonoLoop();
        	break;

        case WPC: //WPC pinball input
        	promptMode("WILLIAMS",32);
        	WPCLoop();
        	break;

        case Stern: //Stern pinball input
        	promptMode("STERN",45);
        	Stern4bitLoop();
        	break;

        case Gottlieb:
        	promptMode("GOTTLIEB",32);
			GottliebLoop();
			break;

        case DataEast:
        	promptMode("DATAEAST",32);
			Stern2bitLoop();
        	break;

        case WhiteStar:
        	promptMode("WHITESTAR",25);
        	Stern2bitLoop();
        	break;

        case WPC95:
			promptMode("WPC95",45);
			WPCLoop();
			break;
        }
	return 0; // compiler warn
}


