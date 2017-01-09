//--------------------------------------------------------------
// File     : stm32_ub_spi1.c
// Datum    : 04.03.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, SPI
// Funktion : SPI-LoLevel-Funktionen (SPI-1)
//
// Hinweis  : mögliche Pinbelegungen
//            SPI1 : SCK :[PA5, PB3] 
//                   MOSI:[PA7, PB5]
//                   MISO:[PA6, PB4]
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_spi1.h"


//--------------------------------------------------------------
// Definition von SPI1
//--------------------------------------------------------------
SPI1_DEV_t SPI1DEV = {
// PORT , PIN      , Clock              , Source 
  {GPIOA,GPIO_Pin_5,RCC_AHB1Periph_GPIOA,GPIO_PinSource5}, // SCK an PA5
  {GPIOA,GPIO_Pin_7,RCC_AHB1Periph_GPIOB,GPIO_PinSource7}, // MOSI an PB5
  {GPIOA,GPIO_Pin_6,RCC_AHB1Periph_GPIOB,GPIO_PinSource6}, // MISO an PB4
};



//--------------------------------------------------------------
// Init von SPI1
// Return_wert :
//  -> ERROR   , wenn SPI schon mit anderem Mode initialisiert
//  -> SUCCESS , wenn SPI init ok war
//--------------------------------------------------------------
ErrorStatus UB_SPI1_Init(SPI1_Mode_t mode)
{
  ErrorStatus ret_wert=ERROR;
  static uint8_t init_ok=0;
  static SPI1_Mode_t init_mode;
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    if(init_mode==mode) ret_wert=SUCCESS;
    return(ret_wert);
  }

  // SPI-Clock enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  // Clock Enable der Pins
  RCC_AHB1PeriphClockCmd(SPI1DEV.SCK.CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI1DEV.MOSI.CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI1DEV.MISO.CLK, ENABLE);

  // SPI Alternative-Funktions mit den IO-Pins verbinden
  GPIO_PinAFConfig(SPI1DEV.SCK.PORT, SPI1DEV.SCK.SOURCE, GPIO_AF_SPI1);
  GPIO_PinAFConfig(SPI1DEV.MOSI.PORT, SPI1DEV.MOSI.SOURCE, GPIO_AF_SPI1);
  GPIO_PinAFConfig(SPI1DEV.MISO.PORT, SPI1DEV.MISO.SOURCE, GPIO_AF_SPI1);

  // SPI als Alternative-Funktion mit PullDown
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

  // SCK-Pin
  GPIO_InitStructure.GPIO_Pin = SPI1DEV.SCK.PIN;
  GPIO_Init(SPI1DEV.SCK.PORT, &GPIO_InitStructure);
  // MOSI-Pin
  GPIO_InitStructure.GPIO_Pin = SPI1DEV.MOSI.PIN;
  GPIO_Init(SPI1DEV.MOSI.PORT, &GPIO_InitStructure);
  // MISO-Pin
  GPIO_InitStructure.GPIO_Pin = SPI1DEV.MISO.PIN;
  GPIO_Init(SPI1DEV.MISO.PORT, &GPIO_InitStructure);

  // SPI-Konfiguration
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  if(mode==SPI_MODE_0) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else if(mode==SPI_MODE_1) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  }else if(mode==SPI_MODE_2) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  }
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI1_VORTEILER;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure); 

  // SPI enable
  SPI_Cmd(SPI1, ENABLE); 

  // init Mode speichern
  init_ok=1;
  init_mode=mode;
  ret_wert=SUCCESS;

  return(ret_wert);
}


//--------------------------------------------------------------
// sendet und empfängt ein Byte per SPI1
// ChipSelect-Signal muss von rufender Funktion gemacht werden
//--------------------------------------------------------------
uint8_t UB_SPI1_SendByte(uint8_t wert)
{ 
  uint8_t ret_wert=0;

  // warte bis senden fertig
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  // byte senden
  SPI_I2S_SendData(SPI1, wert);

  // warte bis empfang fertig
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); 

  // Daten einlesen
  ret_wert=SPI_I2S_ReceiveData(SPI1);

  return(ret_wert);
}
