//--------------------------------------------------------------
// File     : usbd_winusb_core.h
//--------------------------------------------------------------



#ifndef __USB_WINUSB_CORE_H_
#define __USB_WINUSB_CORE_H_


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include  "usbd_ioreq.h"




#define USB_WINUSB_CONFIG_DESC_SIZ       32
#define USB_WINUSB_DESC_SIZ              9

#define WINUSB_DESCRIPTOR_TYPE           0x21


#define WINUSB_REQ_SET_PROTOCOL          0x0B
#define WINUSB_REQ_GET_PROTOCOL          0x03

#define WINUSB_REQ_SET_IDLE              0x0A
#define WINUSB_REQ_GET_IDLE              0x02


//--------------------------------------------------------------
extern USBD_Class_cb_TypeDef  USBD_WINUSB_cb;
extern uint8_t USB_WINUSB_OUT_BUF[];

#endif
