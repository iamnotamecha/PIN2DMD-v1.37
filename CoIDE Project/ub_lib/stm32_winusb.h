//--------------------------------------------------------------
// File     : stm32_winusb.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_WINUSB_H
#define __STM32F4_WINUSB_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "usbd_winusb_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"






//--------------------------------------------------------------
// Anzahl der Bytes die per USB übertragen werden
//  IN  :  CPU --> PC
//  OUT :  CPU <-- PC
//--------------------------------------------------------------
#define   WINUSB_IN_BUFFER_SIZE   64  //  (1...64) Daten zum PC
#define   WINUSB_OUT_BUFFER_SIZE  64  //  (1...64) Daten vom PC
#define   WINUSB_FEATURE_BUFFER_SIZE  4


//--------------------------------------------------------------
// Status der USB-Verbindung
//--------------------------------------------------------------
typedef enum {
  USB_WINUSB_NO_INIT =0, // USB-Schnittstelle noch nicht initialisiert
  USB_WINUSB_DETACHED,   // USB-Verbindung getrennt
  USB_WINUSB_CONNECTED   // USB-Verbindung hergestellt
}USB_WINUSB_STATUS_t;
USB_WINUSB_STATUS_t USB_WINUSB_STATUS; 


//--------------------------------------------------------------
// Status beim Empfangen
//--------------------------------------------------------------
typedef enum {
  RX_USB_ERR =0, // keine USB Verbindung
  RX_EMPTY,      // nichts empfangen
  RX_READY       // es steht was im Empfangspuffer
}USB_WINUSB_RXSTATUS_t; 



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void WINUSB_Init(void);
USB_WINUSB_STATUS_t UB_USB_WINUSB_GetStatus(void);
ErrorStatus UB_USB_WINUSB_SendData(uint8_t *ptr, uint8_t len);


//--------------------------------------------------------------
#endif // __STM32F4_WINUSB_H
