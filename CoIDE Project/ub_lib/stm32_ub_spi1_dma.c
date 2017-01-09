//--------------------------------------------------------------
// File     : stm32_ub_spi1_dma.c
// Datum    : 30.12.2014
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, SPI, DMA, MISC
// Funktion : SPI-LoLevel-Funktionen (SPI-1)
//            (Send und Receive per DMA)
//
// Hinweis  : mögliche Pinbelegungen
//            SPI1 : SCK :[PA5, PB3] 
//                   MOSI:[PA7, PB5]
//                   MISO:[PA6, PB4]
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_spi1_dma.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
ErrorStatus p_init_SPI(SPI1_Mode_t mode);
void p_init_DMA(uint32_t cnt);
void p_init_NVIC(void);



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
// mode : [SPI_MODE_0_MSB] bis [SPI_MODE_3_LSB]
// Return_wert :
//  -> ERROR   , wenn SPI schon mit anderem Mode initialisiert
//  -> SUCCESS , wenn SPI init ok war
//--------------------------------------------------------------
ErrorStatus UB_SPI1_DMA_Init(SPI1_Mode_t mode)
{
  ErrorStatus check;

  check=p_init_SPI(mode);
  if(check==ERROR) return (ERROR);
  p_init_DMA(SPI1_DMA_BUF_SIZE);
  p_init_NVIC();
  P_SPI1_DMA.cnt=0;
  P_SPI1_DMA.status=SPI_READY;

  return(SUCCESS);
}


//--------------------------------------------------------------
// sendet Daten per DMA über SPI1
// ChipSelect-Signal muss von rufender Funktion gemacht werden
// Daten zum senden muessen in "SPI1_DMA.tx_buffer[n]" stehen
// Funktion wartet bis vorherige Daten gesendet wurden
// cnt : Anzahl der Bytes die gesendet werden sollen [1...SPI1_DMA_BUF_SIZE]
//
//  -> ERROR   , wert von cnt falsch
//  -> SUCCESS , Daten werden per DMA gesendet
//--------------------------------------------------------------
ErrorStatus UB_SPI1_DMA_SendBuffer(uint32_t cnt)
{
  uint32_t n;

  if(cnt==0) return(ERROR);
  if(cnt>SPI1_DMA_BUF_SIZE) return(ERROR);

  // warten bis vorheriges senden vom Puffer fertig ist
  while(P_SPI1_DMA.status!=SPI_READY);

  // TX-Puffer kopieren
  for(n=0;n<cnt;n++) {
    P_SPI1_DMA.tx_temp[n]=SPI1_DMA.tx_buffer[n];
  }
  P_SPI1_DMA.cnt=cnt; // anzahl merken

  // status setzen
  P_SPI1_DMA.status=SPI_RUNNING;

  // reinit vom DMA
  p_init_DMA(cnt);

  // DMA-TX enable
  DMA_Cmd(SPI1_DMA_TX_STREAM,ENABLE);
  // DMA-RX enable
  DMA_Cmd(SPI1_DMA_RX_STREAM,ENABLE);
  // DMA-TX request enable
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
  // DMA-RX request enable
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

  // DMA-RX Transfer-Complete Interrupt enable
  DMA_ITConfig(SPI1_DMA_RX_STREAM, DMA_IT_TC, ENABLE);

  // Puffer senden
  SPI_Cmd(SPI1, ENABLE);

  return(SUCCESS);
}


//--------------------------------------------------------------
// Anzahl der Bytes die empfangen wurden auslesen
// return_wert :
//   0   = das empfangen ist noch nicht fertig
//  >0   = empfangene Daten stehen in "SPI1_DMA.rx_buffer[n]"
//--------------------------------------------------------------
uint32_t UB_SPI1_DMA_GetReceivedBytes(void)
{
  if(P_SPI1_DMA.status==SPI_READY) {
    return P_SPI1_DMA.cnt;
  }

  return 0;
}


//--------------------------------------------------------------
// interne Funktion
// init vom SPI-Bus
//--------------------------------------------------------------
ErrorStatus p_init_SPI(SPI1_Mode_t mode)
{
  ErrorStatus ret_wert=ERROR;
  static uint8_t init_ok=0;
  static SPI1_Mode_t init_mode;
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
  uint32_t n;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    if(init_mode==mode) ret_wert=SUCCESS;
    return(ret_wert);
  }

  // Clock enable vom DMA
  RCC_AHB1PeriphClockCmd(SPI_DMA_CLOCK, ENABLE);

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
  if((mode==SPI_MODE_0_MSB) || (mode==SPI_MODE_0_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else if((mode==SPI_MODE_1_MSB) || (mode==SPI_MODE_1_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  }else if((mode==SPI_MODE_2_MSB) || (mode==SPI_MODE_2_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  } 
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI1_VORTEILER;

  if((mode==SPI_MODE_0_MSB) || (mode==SPI_MODE_1_MSB) ||
     (mode==SPI_MODE_2_MSB) || (mode==SPI_MODE_3_MSB)) {
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  }
  else {
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
  }

  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure); 

  // SPI enable
  SPI_Cmd(SPI1, ENABLE);

  // puffer loeschen
  for(n=0;n<SPI1_DMA_BUF_SIZE;n++) {
    SPI1_DMA.tx_buffer[n]=0x00;
    SPI1_DMA.rx_buffer[n]=0x00;
    P_SPI1_DMA.tx_temp[n]=0x00;
    P_SPI1_DMA.rx_temp[n]=0x00;
  }

  // init Mode speichern
  init_ok=1;
  init_mode=mode;
  ret_wert=SUCCESS;

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// init vom DMA
// cnt : Anzahl der Bytes die gesendet werden sollen [1...SPI1_DMA_BUF_SIZE]
//--------------------------------------------------------------
void p_init_DMA(uint32_t cnt)
{
  DMA_InitTypeDef DMA_InitStructure;

  if(cnt==0) return;
  if(cnt>SPI1_DMA_BUF_SIZE) return;

  // DMA deinit
  DMA_DeInit(SPI1_DMA_TX_STREAM);
  DMA_DeInit(SPI1_DMA_RX_STREAM);

  // Config DMA
  DMA_InitStructure.DMA_BufferSize = cnt ;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPI1->DR)) ;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;

  // Config DMA TX
  DMA_InitStructure.DMA_Channel = SPI_DMA_CHANNEL ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
  DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)P_SPI1_DMA.tx_temp;
  DMA_Init(SPI1_DMA_TX_STREAM, &DMA_InitStructure);
  // Config DMA RX
  DMA_InitStructure.DMA_Channel = SPI_DMA_CHANNEL ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
  DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)P_SPI1_DMA.rx_temp;
  DMA_Init(SPI1_DMA_RX_STREAM, &DMA_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void p_init_NVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  // enable RX-Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_DMA_RX_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// ISR vom RX-Stream
// wird bei Transfer-Complete aufgerufen
// wenn alle Daten empfangen worden sind
//--------------------------------------------------------------
void SPI1_DMA_RX_ISR(void)
{
  uint32_t n;

  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(SPI1_DMA_RX_STREAM, SPI1_DMA_RX_TCIF))
  {
    // Flags zuruecksetzen
    DMA_ClearITPendingBit(SPI1_DMA_RX_STREAM, SPI1_DMA_RX_TCIF);

    // RX-Puffer kopieren
    for(n=0;n<P_SPI1_DMA.cnt;n++) {
      SPI1_DMA.rx_buffer[n]=P_SPI1_DMA.rx_temp[n];
    }

    // status zuruecksetzen
    P_SPI1_DMA.status=SPI_READY;
  }
}

