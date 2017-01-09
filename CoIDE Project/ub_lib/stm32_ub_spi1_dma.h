//--------------------------------------------------------------
// File     : stm32_ub_spi1_dma.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SPI1_DMA_H
#define __STM32F4_UB_SPI1_DMA_H



//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "misc.h"



//--------------------------------------------------------------
// Defines der Puffer
//--------------------------------------------------------------
#define  SPI1_DMA_BUF_SIZE   128    // Grösse vom TX- und RX-Puffer in Bytes




//--------------------------------------------------------------
// SPI-Mode
//--------------------------------------------------------------
typedef enum {
  SPI_MODE_0_MSB = 0,  // CPOL=0, CPHA=0 (MSB-First)
  SPI_MODE_1_MSB,      // CPOL=0, CPHA=1 (MSB-First)
  SPI_MODE_2_MSB,      // CPOL=1, CPHA=0 (MSB-First)
  SPI_MODE_3_MSB,      // CPOL=1, CPHA=1 (MSB-First)
  SPI_MODE_0_LSB,      // CPOL=0, CPHA=0 (LSB-First)
  SPI_MODE_1_LSB,      // CPOL=0, CPHA=1 (LSB-First)
  SPI_MODE_2_LSB,      // CPOL=1, CPHA=0 (LSB-First)
  SPI_MODE_3_LSB       // CPOL=1, CPHA=1 (LSB-First) 
}SPI1_Mode_t;


//--------------------------------------------------------------
// SPI-Clock
// Grundfrequenz (SPI1)= APB2 (APB2=84MHz)
// Mögliche Vorteiler = 2,4,8,16,32,64,128,256
//--------------------------------------------------------------

//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_2   // Frq = 42 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_4   // Frq = 21 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_8   // Frq = 10,5 MHz
//#define SPI1_VORTEILER     SPI_BaudRatePrescaler_16  // Frq = 5.25 MHz
#define SPI1_VORTEILER     SPI_BaudRatePrescaler_32  // Frq = 2.625 MHz
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
// Defines vom DMA (siehe Seite 216+217 vom RM0090)
// SPI1_TX = DMA2, Stream3, Channel3 , SPI1_RX = DMA2, Stream0, Channel3
// SPI1_TX = DMA2, Stream5, Channel3 , SPI1_RX = DMA2, Stream2, Channel3
//--------------------------------------------------------------
#define  SPI_DMA_NR              DMA2
#define  SPI_DMA_CLOCK           RCC_AHB1Periph_DMA2
#define  SPI_DMA_CHANNEL         DMA_Channel_3

#define  SPI1_DMA_TX_STREAM      DMA2_Stream3

#define  SPI1_DMA_RX_STREAM      DMA2_Stream0
#define  SPI1_DMA_RX_IRQ         DMA2_Stream0_IRQn
#define  SPI1_DMA_RX_ISR         DMA2_Stream0_IRQHandler
#define  SPI1_DMA_RX_TCIF        DMA_IT_TCIF0


//--------------------------------------------------------------
// Struktur für SPI_DMA
//--------------------------------------------------------------
typedef struct {
  uint8_t tx_buffer[SPI1_DMA_BUF_SIZE]; // TX-Puffer
  uint8_t rx_buffer[SPI1_DMA_BUF_SIZE]; // RX-Puffer
}SPI1_DMA_t;
SPI1_DMA_t SPI1_DMA;



//--------------------------------------------------------------
// Status vom SPI-Bus
//--------------------------------------------------------------
typedef enum {
  SPI_READY =0,        // SPI bereit zum senden
  SPI_RUNNING          // SPI sendet noch
}SPI1_Status_t;



//--------------------------------------------------------------
// interne Struktur für UART_DMA
//--------------------------------------------------------------
typedef struct {
  SPI1_Status_t status;               // status
  uint32_t cnt;                       // anzahl der Daten
  uint8_t tx_temp[SPI1_DMA_BUF_SIZE]; // Zwischenspeicher
  uint8_t rx_temp[SPI1_DMA_BUF_SIZE]; // Zwischenspeicher
}P_SPI1_DMA_t;
P_SPI1_DMA_t P_SPI1_DMA;



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_SPI1_DMA_Init(SPI1_Mode_t mode);
ErrorStatus UB_SPI1_DMA_SendBuffer(uint32_t cnt);
uint32_t UB_SPI1_DMA_GetReceivedBytes(void);




//--------------------------------------------------------------
#endif // __STM32F4_UB_SPI1_DMA_H
