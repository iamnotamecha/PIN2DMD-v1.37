//--------------------------------------------------------------
// File     : usbd_winusb_core.c
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "usbd_winusb_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "stm32_winusb.h"
#include "stm32f4xx_gpio.h"

typedef enum { false, true } bool;

//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_Init (void  *pdev, uint8_t cfgidx);
static uint8_t  USBD_WINUSB_DeInit (void  *pdev, uint8_t cfgidx);
static uint8_t  USBD_WINUSB_Setup (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  *USBD_WINUSB_GetCfgDesc (uint8_t speed, uint16_t *length);
static uint8_t  USBD_WINUSB_DataIn (void  *pdev, uint8_t epnum);
static uint8_t  USBD_WINUSB_DataOut (void  *pdev, uint8_t epnum);

static uint16_t USB_WINUSB_RecData_Len;
static uint8_t USB_WINUSB_RecData_Ready;

extern u8 rxbuf[];
extern bool newpacket;
extern uint16_t rxbuf_ptr;
extern uint8_t displaybuf[];
extern uint8_t rgbbuf[];

uint16_t packetsize;

//--------------------------------------------------------------
USBD_Class_cb_TypeDef  USBD_WINUSB_cb = 
{
  USBD_WINUSB_Init,
  USBD_WINUSB_DeInit,
  USBD_WINUSB_Setup,
  NULL,
  NULL,
  USBD_WINUSB_DataIn,
  USBD_WINUSB_DataOut,
  NULL,
  NULL,
  NULL,      
  USBD_WINUSB_GetCfgDesc,
};


//--------------------------------------------------------------
__ALIGN_BEGIN static uint32_t  USBD_WINUSB_AltSet  __ALIGN_END = 0;
__ALIGN_BEGIN static uint32_t  USBD_WINUSB_Protocol  __ALIGN_END = 0;
__ALIGN_BEGIN static uint32_t  USBD_WINUSB_IdleState __ALIGN_END = 0;


//--------------------------------------------------------------
// USB HID Device Configuration Descriptor
//--------------------------------------------------------------
__ALIGN_BEGIN static uint8_t USBD_WINUSB_CfgDesc[USB_WINUSB_CONFIG_DESC_SIZ] __ALIGN_END =
{
  // CONFIGURATION_DESCRIPTOR
  0x09,								/* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,/* bDescriptorType: Configuration */
  USB_WINUSB_CONFIG_DESC_SIZ,		/* wTotalLength:no of returned bytes */
  0x00,
  0x01,								/* bNumInterfaces: 1 interface */
  0x01,								/* bConfigurationValue: Configuration value */
  0x00,								/* iConfiguration: Index of string descriptor describing the configuration */
  0xE0,								/* bmAttributes: self powered */
  0x32,								/* MaxPower 0 mA */

  // INTERFACE_DESCRIPTOR
  0x09,								/* bLength: Endpoint Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType: */
  0x00,								/* bInterfaceNumber: Number of Interface */
  0x00,								/* bAlternateSetting: Alternate setting */
  0x02,								/* bNumEndpoints: Two endpoints used */
  0xFF,								/* bInterfaceClass: */
  0xFF,								/* bInterfaceSubClass: */
  0xFF,								/* bInterfaceProtocol: */
  0x00,								/* iInterface: */

  // IN Endpoint
  0x07,							/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: Endpoint */
  WINUSB_IN_EP,					/* bEndpointAddress */
  0x02,							/* bmAttributes: Bulk */
  LOBYTE(WINUSB_IN_BUFFER_SIZE),		/* wMaxPacketSize: */
  HIBYTE(WINUSB_IN_BUFFER_SIZE),
  0x00,			/* bInterval: ignore for Bulk transfer */
    	
  // OUT Endpoint
  0x07,
  USB_ENDPOINT_DESCRIPTOR_TYPE,
  WINUSB_OUT_EP,
  0x02,
  LOBYTE(WINUSB_OUT_BUFFER_SIZE),
  HIBYTE(WINUSB_IN_BUFFER_SIZE),
  0x00,

  // Ende (Länge = 32)
};


//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_Init (void  *pdev, 
                               uint8_t cfgidx)
{

  USB_WINUSB_RecData_Ready=0;
  USB_WINUSB_RecData_Len=0;
  
  // Open EP IN (CPU --> PC)
  DCD_EP_Open(pdev,
              WINUSB_IN_EP,
              WINUSB_IN_BUFFER_SIZE,
              USB_OTG_EP_INT);
  
  // Open EP OUT (CPU <-- PC)
  DCD_EP_Open(pdev,
              WINUSB_OUT_EP,
              WINUSB_OUT_BUFFER_SIZE,
              USB_OTG_EP_INT);

  // Prepare Out endpoint to receive next packet (CPU <-- PC)
  DCD_EP_PrepareRx(pdev,
              WINUSB_OUT_EP,
              (uint8_t*)(USB_WINUSB_OUT_BUF),
              WINUSB_OUT_BUFFER_SIZE);
  
  return USBD_OK;
}


//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , WINUSB_IN_EP);
  DCD_EP_Close (pdev , WINUSB_OUT_EP);
  
  
  return USBD_OK;
}


//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case WINUSB_REQ_SET_PROTOCOL:
      USBD_WINUSB_Protocol = (uint8_t)(req->wValue);
      break;
      
    case WINUSB_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_WINUSB_Protocol,
                        1);    
      break;
      
    case WINUSB_REQ_SET_IDLE:
      USBD_WINUSB_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case WINUSB_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_WINUSB_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
       if( req->wValue >> 8 == WINUSB_DESCRIPTOR_TYPE)
      {        
        pbuf = USBD_WINUSB_CfgDesc + 0x12;
        len = MIN(USB_WINUSB_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_WINUSB_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_WINUSB_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

//--------------------------------------------------------------
static uint8_t  *USBD_WINUSB_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_WINUSB_CfgDesc);
  return USBD_WINUSB_CfgDesc;
}


//--------------------------------------------------------------
// wird aufgerufen, wenn der PC Daten von der CPU abholt
// (IN  :  CPU --> PC)
//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_DataIn (void  *pdev, uint8_t epnum)
{
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, WINUSB_IN_EP);
  return USBD_OK;
}


//--------------------------------------------------------------
// wird aufgerufem, wenn der PC Daten zur CPU sendet
// (OUT :  CPU <-- PC)
//--------------------------------------------------------------
static uint8_t  USBD_WINUSB_DataOut (void *pdev, uint8_t epnum)
{
  uint16_t data_cnt;
 
  if (epnum == WINUSB_OUT_EP)
  {    
    data_cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
    if (((USB_OTG_CORE_HANDLE*)pdev)->dev.device_status == USB_OTG_CONFIGURED )
    {
      // [UB] known Warning :
      // passing argument 2 of 'USB_OTG_ReadPacket' makes pointer from integer without a cast [enabled by default]
      USB_OTG_ReadPacket((USB_OTG_CORE_HANDLE*)pdev, *USB_WINUSB_OUT_BUF, WINUSB_OUT_BUFFER_SIZE);
      USB_WINUSB_RecData_Len=data_cnt;
      USB_WINUSB_RecData_Ready=1;

      DCD_EP_PrepareRx(pdev, WINUSB_OUT_EP, (uint8_t*)(USB_WINUSB_OUT_BUF), WINUSB_OUT_BUFFER_SIZE);
      if ((USB_WINUSB_OUT_BUF[0]== 0x81) & (USB_WINUSB_OUT_BUF[1] == 0xc3) & (USB_WINUSB_OUT_BUF[2] == 0xe7)) {
      	  rxbuf_ptr = 0;
      	  packetsize = 2052;}
        if ((USB_WINUSB_OUT_BUF[0]== 0x81) & (USB_WINUSB_OUT_BUF[1] == 0xc3) & (USB_WINUSB_OUT_BUF[2] == 0xe8)) {
      	  rxbuf_ptr = 0;
      	  packetsize = 16388;}
          memcpy(rxbuf+rxbuf_ptr,USB_WINUSB_OUT_BUF,USB_WINUSB_RecData_Len);
        rxbuf_ptr = rxbuf_ptr + USB_WINUSB_RecData_Len;
        if (rxbuf_ptr >= packetsize) {
        	  rxbuf_ptr = 0;
        	  if (packetsize == 2052) {
        		  newpacket = true;
        	  } else {
        		  GPIO_ToggleBits(GPIOD,GPIO_Pin_15); //toggle blue LED
        		  memcpy(rgbbuf,rxbuf+4,16384);
        	  }
        }

    }
  }
  return USBD_OK;
}


//--------------------------------------------------------------
// zum senden von Daten an den PC
//--------------------------------------------------------------
uint8_t USBD_WINUSB_SendReport     (USB_OTG_CORE_HANDLE  *pdev,
                                 uint8_t *report,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, WINUSB_IN_EP, report, len);
  }
  else return USBD_FAIL;

  return USBD_OK;
}


