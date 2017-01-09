//--------------------------------------------------------------
// File     : usbd_desc.c
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "usb_regs.h"


#define USBD_VID                        0x0314  // VID
#define USBD_PID                        0xe457  // PID
#define USBD_VER                        0x0200  // Version


#define USBD_LANGID_STRING              0x409
#define USBD_MANUFACTURER_STRING        "PIN2DMD"

#define USBD_PRODUCT_FS_STRING          "PIN2DMD"
#define USBD_SERIALNUMBER_FS_STRING     "00000000050C"

#define USBD_CONFIGURATION_FS_STRING    "WINUSB Config"
#define USBD_INTERFACE_FS_STRING        "WINUSB Interface"


USBD_DEVICE USR_desc =
{
  USBD_USR_DeviceDescriptor,
  USBD_USR_LangIDStrDescriptor, 
  USBD_USR_ManufacturerStrDescriptor,
  USBD_USR_ProductStrDescriptor,
  USBD_USR_SerialStrDescriptor,
  USBD_USR_ConfigStrDescriptor,
  USBD_USR_InterfaceStrDescriptor,
  USBD_USR_OSStrDescriptor,
  USBD_USR_ExtPropertiesFeatureDescriptor,
  USBD_USR_ExtCompatIDFeatureDescriptor
};


// USBD_DeviceDesc
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END =
{
    0x12,						/*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,	/*bDescriptorType*/
    0x00,						/*bcdUSB */
    0x02,
    0x00,						/*bDeviceClass*/
    0x00,						/*bDeviceClass*/
    0x00,						/*bDeviceClass*/
    USB_OTG_MAX_EP0_SIZE,
    LOBYTE(USBD_VID),			/*idVendor*/
    HIBYTE(USBD_VID),			/*idVendor*/
    LOBYTE(USBD_PID),			/*idVendor*/
    HIBYTE(USBD_PID),			/*idVendor*/
    LOBYTE(USBD_VER),
    HIBYTE(USBD_VER),			/*bcdDevice rel. 2.00*/
    USBD_IDX_MFC_STR,			/*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR,		/*Index of product string*/
    USBD_IDX_SERIAL_STR,		/*Index of serial number string*/
    USBD_CFG_MAX_NUM			/*bNumConfigurations*/
};


// USBD_DeviceQualifierDesc
__ALIGN_BEGIN uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};


// USBD_LangIDDesc
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END =
{
     USB_SIZ_STRING_LANGID,         
     USB_DESC_TYPE_STRING,       
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING), 
};

// Extended Compat ID OS Feature Descriptor
__ALIGN_BEGIN uint8_t USBD_OSStrDesc[USB_LEN_OS_DESC] __ALIGN_END =
{
USB_LEN_OS_DESC,
USB_DESC_TYPE_STRING,
OS_STRING,
MS_VENDORCODE,
0x00
};




typedef struct
{
// Header
uint32_t dwLength;
uint16_t  bcdVersion;
uint16_t  wIndex;
uint8_t  bCount;
uint8_t  bReserved1[7];
// Function Section 1
uint8_t  bFirstInterfaceNumber;
uint8_t  bReserved2;
uint8_t  bCompatibleID[8];
uint8_t  bSubCompatibleID[8];
uint8_t  bReserved3[6];
} USBD_CompatIDDescStruct;

USBD_CompatIDDescStruct USBD_CompatIDDesc =
{ sizeof(USBD_CompatIDDesc),
		0x0100,
		0x0004,
		0x01,
		{0},
		0x00, 				//bFirstInterfaceNumber: the WinUSB interface in this firmware is interface #0
        0x01,               //Reserved - fill this reserved byte with 0x01 according to documentation
		"WINUSB",//compatID - "WINUSB" (with two null terminators to fill all 8 bytes)
		{0},                  //subCompatID - eight bytes of 0
		{0}                       //Reserved};
};

// Extended Properties OS Feature Descriptor
typedef struct
{
// Header
uint32_t dwLength;
uint16_t  bcdVersion;
uint16_t  wIndex;
uint16_t  wCount;
// Custom Property Section 1
uint32_t dwSize;
uint32_t dwPropertyDataType;
uint16_t  wPropertyNameLength;
uint16_t  bPropertyName[20];
uint32_t dwPropertyDataLength;
uint16_t  bPropertyData[39];
} USBD_ExtPropertiesDescStruct;

USBD_ExtPropertiesDescStruct USBD_ExtPropertiesDesc =
{
		sizeof(USBD_ExtPropertiesDesc),
		0x0100,
		0x0005,
		0x0001,\
		0x00000084,
		0x00000001,\
		0x0028,
		{'D','e','v','i','c','e','I','n','t','e','r','f','a','c','e','G','U','I','D',0},\
		0x0000004E,
		{'{','E','4','9','E','0','B','E','E','-','3','7','1','4','-','4','9','F','5','-','A','3','7','6','-','A','3','A','C','6','A','9','7','8','A','F','C','}',0}
};


//--------------------------------------------------------------
uint8_t *  USBD_USR_DeviceDescriptor( uint8_t speed , uint16_t *length)
{
  *length = sizeof(USBD_DeviceDesc);
  return USBD_DeviceDesc;
}

//--------------------------------------------------------------
uint8_t *  USBD_USR_LangIDStrDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_LangIDDesc);  
  return USBD_LangIDDesc;
}


//--------------------------------------------------------------
uint8_t *  USBD_USR_ProductStrDescriptor( uint8_t speed , uint16_t *length)
{

    USBD_GetString ((uint8_t*)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

//--------------------------------------------------------------
uint8_t *  USBD_USR_ManufacturerStrDescriptor( uint8_t speed , uint16_t *length)
{
  USBD_GetString ((uint8_t*)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

//--------------------------------------------------------------
uint8_t *  USBD_USR_SerialStrDescriptor( uint8_t speed , uint16_t *length)
{

    USBD_GetString ((uint8_t*)USBD_SERIALNUMBER_FS_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

//--------------------------------------------------------------
uint8_t *  USBD_USR_ConfigStrDescriptor( uint8_t speed , uint16_t *length)
{

    USBD_GetString ((uint8_t*)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);

  return USBD_StrDesc;  
}


//--------------------------------------------------------------
uint8_t *  USBD_USR_InterfaceStrDescriptor( uint8_t speed , uint16_t *length)
{

    USBD_GetString ((uint8_t*)USBD_INTERFACE_FS_STRING, USBD_StrDesc, length);

  return USBD_StrDesc;  
}

uint8_t *  USBD_USR_OSStrDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_OSStrDesc);
  return USBD_OSStrDesc;
}

uint8_t *  USBD_USR_ExtCompatIDFeatureDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_CompatIDDesc);
  return (uint8_t*)&USBD_CompatIDDesc;
}

uint8_t *  USBD_USR_ExtPropertiesFeatureDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_ExtPropertiesDesc);
  return (uint8_t*)&USBD_ExtPropertiesDesc;
}




