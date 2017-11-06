/*
 * descriptor.c
 *
 *  Created on: 25 окт. 2017 г.
 *      Author: valeriy
 */
#include "stm32f10x.h"
#define STRING_DT			3

const uint8_t DeviceDescriptor[] = { 0x12, /* bLength */
1, /* bDescriptorType */
0x00, 0x02, /* bcdUSB = 2.00 */
0x02, /* bDeviceClass: CDC */
0x00, /* bDeviceSubClass */
0x00, /* bDeviceProtocol */
0x08, /* bMaxPacketSize0 */
0x83, 0x04, /* idVendor = 0x0483 */
0x40, 0x57, /* idProduct = 0x7540 */
0x00, 0x02, /* bcdDevice = 2.00 */
1, /* Index of string descriptor describing manufacturer */
2, /* Index of string descriptor describing product */
3, /* Index of string descriptor describing the device's serial number */
0x01 /* bNumConfigurations */
};
const uint8_t ConfigDescriptor[] = { 9, 2, 67, 00, 2, 1, 0, 0xc0, 0x32,

0x09, /* bLength: Interface Descriptor size */
4, /* bDescriptorType: Interface */
/* Interface descriptor type */
0x00, /* bInterfaceNumber: Number of Interface */
0x00, /* bAlternateSetting: Alternate setting */
0x01, /* bNumEndpoints: One endpoints used */
0x02, /* bInterfaceClass: Communication Interface Class */
0x02, /* bInterfaceSubClass: Abstract Control Model */
0x01, /* bInterfaceProtocol: Common AT commands */
0x00, /* iInterface: */
/*Header Functional Descriptor*/
0x05, /* bLength: Endpoint Descriptor size */
0x24, /* bDescriptorType: CS_INTERFACE */
0x00, /* bDescriptorSubtype: Header Func Desc */
0x10, /* bcdCDC: spec release number */
0x01,
/*Call Management Functional Descriptor*/
0x05, /* bFunctionLength */
0x24, /* bDescriptorType: CS_INTERFACE */
0x01, /* bDescriptorSubtype: Call Management Func Desc */
0x00, /* bmCapabilities: D0+D1 */
0x01, /* bDataInterface: 1 */
/*ACM Functional Descriptor*/
0x04, /* bFunctionLength */
0x24, /* bDescriptorType: CS_INTERFACE */
0x02, /* bDescriptorSubtype: Abstract Control Management desc */
0x02, /* bmCapabilities */
/*Union Functional Descriptor*/
0x05, /* bFunctionLength */
0x24, /* bDescriptorType: CS_INTERFACE */
0x06, /* bDescriptorSubtype: Union func desc */
0x00, /* bMasterInterface: Communication class interface */
0x01, /* bSlaveInterface0: Data Class Interface */
/*Endpoint 2 Descriptor*/
0x07, /* bLength: Endpoint Descriptor size */
5, /* bDescriptorType: Endpoint */
0x82, /* bEndpointAddress: (IN2) */
0x03, /* bmAttributes: Interrupt */
8, /* wMaxPacketSize: */
0x00, 0xFF, /* bInterval: */

// end config; start interface descriptor
		9,// size of descriptor
		4, 1, 0, 2,		// endpoints
		0x0a,		// interface class
		2,		// interface subclass
		0,		// interface protocol Common AT commands
		0,		// interface string index

		7,		// size endpoint descriptor
		5,		// endpoint descriptor type
		0x81,	// IN endpoint
		2, 64, 00, 00,

		7,		// endpoint OUT
		5, 3, 2, 16, 00, 00 };
/*
 const uint8_t Virtual_Com_Port_ConfigDescriptor[] =
 {
 Configuration Descriptor
 0x09,    bLength: Configuration Descriptor size
 USB_CONFIGURATION_DESCRIPTOR_TYPE,       bDescriptorType: Configuration
 VIRTUAL_COM_PORT_SIZ_CONFIG_DESC,        wTotalLength:no of returned bytes
 0x00,
 0x02,    bNumInterfaces: 2 interface
 0x01,    bConfigurationValue: Configuration value
 0x00,    iConfiguration: Index of string descriptor describing the configuration
 0xC0,    bmAttributes: self powered
 0x32,    MaxPower 0 mA
 Interface Descriptor
 0x09,    bLength: Interface Descriptor size
 USB_INTERFACE_DESCRIPTOR_TYPE,   bDescriptorType: Interface
 Interface descriptor type
 0x00,    bInterfaceNumber: Number of Interface
 0x00,    bAlternateSetting: Alternate setting
 0x01,    bNumEndpoints: One endpoints used
 0x02,    bInterfaceClass: Communication Interface Class
 0x02,    bInterfaceSubClass: Abstract Control Model
 0x01,    bInterfaceProtocol: Common AT commands
 0x00,    iInterface:
 Header Functional Descriptor
 0x05,    bLength: Endpoint Descriptor size
 0x24,    bDescriptorType: CS_INTERFACE
 0x00,    bDescriptorSubtype: Header Func Desc
 0x10,    bcdCDC: spec release number
 0x01,
 Call Management Functional Descriptor
 0x05,    bFunctionLength
 0x24,    bDescriptorType: CS_INTERFACE
 0x01,    bDescriptorSubtype: Call Management Func Desc
 0x00,    bmCapabilities: D0+D1
 0x01,    bDataInterface: 1
 ACM Functional Descriptor
 0x04,    bFunctionLength
 0x24,    bDescriptorType: CS_INTERFACE
 0x02,    bDescriptorSubtype: Abstract Control Management desc
 0x02,    bmCapabilities
 Union Functional Descriptor
 0x05,    bFunctionLength
 0x24,    bDescriptorType: CS_INTERFACE
 0x06,    bDescriptorSubtype: Union func desc
 0x00,    bMasterInterface: Communication class interface
 0x01,    bSlaveInterface0: Data Class Interface
 Endpoint 2 Descriptor
 0x07,    bLength: Endpoint Descriptor size
 USB_ENDPOINT_DESCRIPTOR_TYPE,    bDescriptorType: Endpoint
 0x82,    bEndpointAddress: (IN2)
 0x03,    bmAttributes: Interrupt
 VIRTUAL_COM_PORT_INT_SIZE,       wMaxPacketSize:
 0x00,
 0xFF,    bInterval:
 Data class interface descriptor
 0x09,    bLength: Endpoint Descriptor size
 USB_INTERFACE_DESCRIPTOR_TYPE,   bDescriptorType:
 0x01,    bInterfaceNumber: Number of Interface
 0x00,    bAlternateSetting: Alternate setting
 0x02,    bNumEndpoints: Two endpoints used
 0x0A,    bInterfaceClass: CDC
 0x00,    bInterfaceSubClass:
 0x00,    bInterfaceProtocol:
 0x00,    iInterface:
 Endpoint 3 Descriptor
 0x07,    bLength: Endpoint Descriptor size
 USB_ENDPOINT_DESCRIPTOR_TYPE,    bDescriptorType: Endpoint
 0x03,    bEndpointAddress: (OUT3)
 0x02,    bmAttributes: Bulk
 VIRTUAL_COM_PORT_DATA_SIZE,              wMaxPacketSize:
 0x00,
 0x00,    bInterval: ignore for Bulk transfer
 Endpoint 1 Descriptor
 0x07,    bLength: Endpoint Descriptor size
 USB_ENDPOINT_DESCRIPTOR_TYPE,    bDescriptorType: Endpoint
 0x81,    bEndpointAddress: (IN1)
 0x02,    bmAttributes: Bulk
 VIRTUAL_COM_PORT_DATA_SIZE,              wMaxPacketSize:
 0x00,
 0x00     bInterval
 };
 */

/* USB String Descriptors */
const uint8_t StringLangID[4] = { 4,
STRING_DT, 0x09, 0x04 /* LangID = 0x0409: U.S. English */
};

const uint8_t StringVendor[38] = { 38, /* Size of Vendor string */
STRING_DT, /* bDescriptorType*/
/* Manufacturer: "STMicroelectronics" */
'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0, 'l', 0, 'e', 0,
		'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0, 'c', 0, 's', 0 };

const uint8_t StringProduct[50] = { 46, /* bLength */
STRING_DT, /* bDescriptorType */
/* Product name: "STM32 Virtual COM Port" */
'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0,
		'u', 0, 'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0,
		'o', 0, 'r', 0, 't', 0 //, ' ', 0, ' ', 0
		};

const uint8_t StringSerial[26] = { 26, /* bLength */
STRING_DT, /* bDescriptorType */
'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, 'S', 0, 'V', 0, 'A', 0, 'H', 0, 'A', 0,
		'R', 0, 'D', 0 };

