#include <avr/io.h>
#include "avrUsbUtils.h"

uint8_t UsbWrite(uint8_t endpoint, uint8_t* data, uint8_t length)
{
	int i = 0;

	int interrupt = SREG & 0x80; //store global interrupt flag
	SREG = SREG & 0x7F; //disable interrupts

	UENUM = endpoint;

	if ( USB_READY && USB_CAN_WRITE)
	{
		for( i = 0; (i < length) && (USB_HAS_SPACE); i++ )
		{
//			UENUM = endpoint; //setting this everytime prevents data corruption
			//I suspect there is an interrupt that is firing and screwing up UENUM
			UEDATX = data[i];
		}

		USB_SEND;
	}

	SREG |= interrupt; //restore interrputs

	return i;
}

uint8_t UsbWriteBlocking(uint8_t endpoint, uint8_t* data, uint8_t length)
{
	int i = 0;

	int interrupt = SREG & 0x80; //store global interrupt flag
	SREG = SREG & 0x7F; //disable interrupts

	UENUM = endpoint;

	while ( !(USB_READY && USB_CAN_WRITE) );

	for( i = 0; (i < length) && (USB_HAS_SPACE); i++ )
		UEDATX = data[i];

	USB_SEND;

	SREG |= interrupt; //restore interrputs

	return i;
}

uint8_t UsbRead(uint8_t endpoint, uint8_t* data, uint8_t maxlength)
{
	int i = 0;

	int interrupt = SREG & 0x80; //store global interrupt flag
	SREG = SREG & 0x7F; //disable interrupts

	UENUM = endpoint;

	while ( !(USB_READY) );

	UEINTX &= ~_BV(RXOUTI);

	for( i = 0; (i < maxlength) && (USB_HAS_SPACE); i++ )
		data[i] = UEDATX;

	UEINTX &= ~_BV(FIFOCON);

	SREG |= interrupt; //restore interrputs

	return i;
}


/*

Copyright (c) 2012, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
