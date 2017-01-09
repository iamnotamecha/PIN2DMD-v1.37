//--------------------------------------------------------------
// File     : stm32_winusb.c
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_winusb.h"



//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
USB_OTG_CORE_HANDLE  USB_OTG_dev;
uint8_t USB_WINUSB_OUT_BUF[WINUSB_OUT_BUFFER_SIZE];  // CPU <-- PC
uint8_t USB_WINUSB_IN_BUF[WINUSB_IN_BUFFER_SIZE];    // CPU --> PC




//--------------------------------------------------------------
// Init vom USB-OTG-Port als WINUSB-Device
//--------------------------------------------------------------
void WINUSB_Init(void)
{
  USB_WINUSB_STATUS=USB_WINUSB_DETACHED;
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_WINUSB_cb, &USR_cb);
}



//--------------------------------------------------------------
// auslesen vom Status der USB-Schnittstelle
// Return_wert :
//  -> USB_WINUSB_NO_INIT   , USB-Schnittstelle noch nicht initialisiert
//  -> USB_WINUSB_DETACHED  , USB-Verbindung getrennt
//  -> USB_WINUSB_CONNECTED , USB-Verbindung hergestellt
//--------------------------------------------------------------
USB_WINUSB_STATUS_t WINUSB_GetStatus(void)
{
  return(USB_WINUSB_STATUS);
}



//--------------------------------------------------------------
// Sendet Daten per USB-OTG-Schnittstelle (CPU --> PC)
// len = Anzahl der Bytes zum senden (1...WINUSB_IN_BUFFER_SIZE)
// Return_wert :
//  -> ERROR   , wenn Daten nicht gesendet wurde
//  -> SUCCESS , wenn Daten gesendet wurde
//--------------------------------------------------------------
ErrorStatus WINUSB_SendData(uint8_t *ptr, uint8_t len)
{
  uint8_t n;

  if(USB_WINUSB_STATUS!=USB_WINUSB_CONNECTED) {
    // senden nur, wenn Verbindung hergestellt ist
    return(ERROR);
  }

  // Puffer kopieren
  for(n=0;n<WINUSB_IN_BUFFER_SIZE;n++) {
    if(n<len) {
      USB_WINUSB_IN_BUF[n]=*ptr;
      ptr++;
    }
    else {
      // der Rest vom Puffer wird mit 0x00 gefüllt
      USB_WINUSB_IN_BUF[n]=0x00;
    }
  }

  // Daten senden
    if(USBD_WINUSB_SendReport (&USB_OTG_dev, USB_WINUSB_IN_BUF, WINUSB_IN_BUFFER_SIZE)!=USBD_OK) {
      return(ERROR);
    }

  return(SUCCESS);
}

