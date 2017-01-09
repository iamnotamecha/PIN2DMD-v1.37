//--------------------------------------------------------------
// File     : stm32_ub_spi1.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SPI1_H
#define __STM32F4_UB_SPI1_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"


//--------------------------------------------------------------
// SPI-Mode
//--------------------------------------------------------------
typedef enum {
  SPI_MODE_0 = 0,  // CPOL=0, CPHA=0
  SPI_MODE_1,      // CPOL=0, CPHA=1
  SPI_MODE_2,      // CPOL=1, CPHA=0
  SPI_MODE_3       // CPOL=1, CPHA=1 
}SPI1_Mode_t;


//--------------------------------------------------------------
// SPI-Clock
// Grundfrequenz (SPI1)= APB2 (APB2=84MHz)
// Mögliche Vorteiler = 2,4,8,16,32,64,128,256
//--------------------------------------------------------------

//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_2   // Frq = 42 MHz
#define SPI1_VORTEILER     SPI_BaudRatePrescaler_4   // Frq = 21 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_8   // Frq = 10,5 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_16  // Frq = 5.25 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_32  // Frq = 2.625 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_64  // Frq = 1.3125 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_128 // Frq = 656.2 kHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_256 // Frq = 328.1 kHz




//--------------------------------------------------------------
// Struktur eines SPI-Pins
//--------------------------------------------------------------
typedef struct {
  GPIO_TypeDef* PORT;     // Port
  const uint16_t PIN;     // Pin
  const uint32_t CLK;     // Clock
  const uint8_t SOURCE;   // Source
}SPI1_PIN_t; 

//--------------------------------------------------------------
// Struktur vom SPI-Device
//--------------------------------------------------------------
typedef struct {
  SPI1_PIN_t  SCK;        // SCK-Pin
  SPI1_PIN_t  MOSI;       // MOSI-Pin
  SPI1_PIN_t  MISO;       // MISO-Pin
}SPI1_DEV_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_SPI1_Init(SPI1_Mode_t mode);
uint8_t UB_SPI1_SendByte(uint8_t wert);




//--------------------------------------------------------------
#endif // __STM32F4_UB_SPI1_H
