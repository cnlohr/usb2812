#ifndef AVRUSBUTILS_H
#define AVRUSBUTILS_H

#include <stdint.h>

#define USB_READY		(UEINTX & (1<<FIFOCON))
#define USB_CAN_WRITE		(UEINTX & (1<<TXINI))
//#define USB_WAIT_TXINI		while ( !USB_CAN_WRITE )
#define USB_HAS_SPACE		UEINTX & (1<<RWAL)
#define USB_SEND 		UEINTX = 0x7A
#define USB_KILL 		UEINTX = 0xDF

uint8_t UsbWrite(uint8_t endpoint, uint8_t* data, uint8_t length);
uint8_t UsbWriteBlocking(uint8_t endpoint, uint8_t* data, uint8_t length);

uint8_t UsbRead(uint8_t endpoint, uint8_t* data, uint8_t maxlength);

#endif

/*

Copyright (c) 2012, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
