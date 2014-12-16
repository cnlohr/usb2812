#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usb.h"
#include "SPIPrinting.h"
#include <stdio.h>

//unsigned char sendbuffer[128];
//unsigned char recvbuffer[32];

int main()
{
	unsigned char i;

	cli();
	DDRD = 1;
	SetupPrintf();

	USB_ZeroPrescaler();
	USB_Init();

	//Safe to leave lockdown...
	sei();

/*	for( i = 0; i < 128; i++ )
	{
		sendbuffer[i] = 0x30 + i;
	}*/

	_delay_ms(500);
	SPISendStr( "Tick\n" );
	_delay_ms(500);
	sendhex2( USBInitState );
	while( USBInitState <= 1 );
	while(1) { 
		//Freedom! Could probably sleep here.
	}

fail:
	//Should, in the future, reset the device.
	_delay_ms(10);
	SPISendStr( "failure\n" );
	while(1);

	return 0;
}

/* 
 * Copyright 2011 Charles Lohr
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

