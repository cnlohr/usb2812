#ifndef _USBCONFIG_H
#define _USBCONFIG_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

//ONLY INCLUDE THIS IN USB.C!

//General defines.

#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40

#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06
#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

#define MAX_ENDPOINT		4

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)
// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
// CDC (communication class device)
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22




// You can change these to give your code its own name.
#define STR_MANUFACTURER	L"[mfg]"
#define STR_PRODUCT		L"[prod]"
#define STR_SERIAL      L"[ser]"
#define VENDOR_ID		0xabcd
#define PRODUCT_ID		0xf003

//#define HARDWARE_INTERFACE 0

#define ENDPOINT0_SIZE 64
//#define ENDPOINT2_SIZE 32
//#define ENDPOINT3_SIZE 64
//#define ENDPOINT4_SIZE 64

//Endpoints 1, 2, 3, and 4.  Note: Only 3 and 4 may be double buffered.
//0 indicates no endpoint here, 1 indicates an endpoint.
//Interrupt types IN mean TO HOST, OUT mean TO AVR
//Size may be 64, 32, 16 or 8, ONLY.
static const uint8_t PROGMEM endpoint_config_table[] = {
	0,
	0, //1, EP_TYPE_ISOCHRONOUS_OUT,  EP_SIZE(ENDPOINT2_SIZE) | EP_SINGLE_BUFFER,
	0,
	0,
};


static const uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x00,				// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0xcd, 0xab,				// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	3,					// iSerialNumber
	1					// bNumConfigurations
};

#define CONFIG_DESCRIPTOR_SIZE (9+9+0)

static const uint8_t PROGMEM config_descriptor[] = {
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LSB(CONFIG_DESCRIPTOR_SIZE),			// wTotalLength
	MSB(CONFIG_DESCRIPTOR_SIZE),
	1,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0xC0,					// bmAttributes
	50,					// bMaxPower

	9,					// bLength
	4,					// bDescriptorType
	0,			// bInterfaceNumber (unused, would normally be used for HID)
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	0xff,					// bInterfaceClass (0xff = vendor-specific)
	0x01,					// bInterfaceSubClass
	0x01,					// bInterfaceProtocol
	0,					// iInterface
/*
	7,					// bLength
	5,					// bDescriptorType
	2,		// bEndpointAddress
	0x01,					// bmAttributes
	ENDPOINT2_SIZE, 0,			// wMaxPacketSize
	1,					// bInterval
*/

};



// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.
struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};
static const struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};
static const struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
static const struct usb_string_descriptor_struct PROGMEM string3 = {
	sizeof(STR_SERIAL),
	3,
	STR_SERIAL
};


// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM const descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
//	{0x2200, KEYBOARD_INTERFACE, keyboard_hid_report_desc, sizeof(keyboard_hid_report_desc)},
//	{0x2100, KEYBOARD_INTERFACE, config1_descriptor+KEYBOARD_HID_DESC_OFFSET, 9},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},	
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))




#endif

/*
 * Copyright 2011-2012 Charles Lohr
 *   -- based off of --
 * USB Keyboard Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

