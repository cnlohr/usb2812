
#include "usb.h"
#include "SPIPrinting.h"
#include "usbconfig.h"
#include <stdio.h>
#include <avr/delay.h>


static uint8_t buf[64];

#define NOOP asm volatile("nop" ::)

//unsigned char USB_Initialized;
volatile char USBInitState;

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;

void USB_ZeroPrescaler()
{
        CLKPR=0x80;
        CLKPR=0x00;
}

//Hacky mcHackerson:
//Reroute PC5, PCINT9, OC.1B to itself.
//Alternatively: OC.0B to itself.
//volatile register uint8_t speedytail asm("r4");

/*
ISR( PCINT1_vect, ISR_NAKED )
{
	speedytail++;
	reti();
}
*/
/*
ISR( INT0_vect, ISR_NAKED )
{
	speedytail++;
	reti();
}
*/

void USB_Init()
{
	USBInitState = -1;
	USBCON = (1<<USBE)|(1<<FRZCLK); //USB "Freeze" (And enable)
	REGCR = 0; //enable regulator.
	PLLCSR = (1<<PLLE)|(1<<PLLP0); //PLL Config
        while (!(PLLCSR & (1<<PLOCK)));	// wait for PLL lock
	USBCON = 1<<USBE; //Unfreeze USB
        UDCON = 0;// enable attach resistor (go on bus)
        UDIEN = (1<<EORSTE)|(1<<SOFE);

/*
	speedytail = 5;

	EICRA = 0; //low level generates request.
	EIMSK |= _BV(INT0);

	TCCR0A = _BV(COM0B0) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
	TCCR0B = _BV(CS01) | _BV(WGM02); // /8
	OCR0A = 4;
	OCR0B = 2;
*/

/*
//For PCINT9 on OC1B
	DDRC |= _BV(5);
	//Hacky interrupt.
	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT9);

	ICR1 = 2;
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); //16/8 = 2MHz clock
	TCCR1A = _BV(WGM11) | _BV(COM1B0) | _BV(COM1B1); //set on OC match, clear on TOP
	OCR1B = 0;
*/
}


// USB Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame
//

int triggerep = 0;

ISR(USB_GEN_vect)
{
	uint8_t intbits;
	intbits = UDINT;
	UDINT = 0;
	if (intbits & (1<<EORSTI)) {
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1<<RXSTPE);
		usb_configuration = 0;
		USBInitState = 1;
	}
}



// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1<<RXOUTI);
}




// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR(USB_COM_vect)
{
//    UDIEN = 0;//(1<<EORSTE)|(1<<SOFE);
//	sei();


	static unsigned char frame;
	uint8_t intbits;
	const uint8_t *list;
	const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;
restart_usb:

	UENUM = 0;
	intbits = UEINTX;

	SPIPutChar( 'x' );	

	if (intbits & (1<<RXSTPI)) {
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);

		UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
		if (bRequest == GET_DESCRIPTOR)
		{
			list = (const uint8_t *)descriptor_list;
			for (i=0; ; i++)
			{
				if (i >= NUM_DESC_LIST)
				{
					UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
					goto end;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue)
				{
					list += sizeof(struct descriptor_list_struct);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex)
				{
					list += sizeof(struct descriptor_list_struct)-2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length) len = desc_length;
			do
			{
				// wait for host ready for IN packet
				do
				{
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) goto end;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--)
				{
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);

			goto end;
		}
		else if (bRequest == SET_ADDRESS)
		{
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			goto end;
		}
		else if (bRequest == SET_CONFIGURATION && bmRequestType == 0)
		{
			usb_configuration = wValue;
			usb_send_in();
			cfg = endpoint_config_table;
			for (i=1; i<5; i++)
			{
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en)
				{
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
			UERST = 0x1E;
			UERST = 0;
			USBInitState = 2;
			goto end;
		}
		else if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80)
		{
			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			goto end;
		}
		else if (bRequest == GET_STATUS)
		{
			usb_wait_in_ready();
			i = 0;
			#ifdef SUPPORT_ENDPOINT_HALT
			if (bmRequestType == 0x82)
			{
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			#endif
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			goto end;
		}
		else if( bRequest == 0xA3 ) //My request/Device->Host
		{
			// wait for host ready for IN packet
			do {
				i = UEINTX;
			} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
			if (i & (1<<RXOUTI)) goto end;	// abort

			n = 11;
			for (i = n; i; i--)
			{
				UEDATX = i;
			}
			usb_send_in();

			//printf( "0xA3 / data:\n" );
			SPIPutChar( 'X' );
			SPIPutChar( '\n' );
		}
		else if( bRequest == 0xA4 ) //Control-In (Us to CPU)
		{
			usb_wait_in_ready();
			UEDATX = 4;UEDATX = 5;UEDATX = 6;UEDATX = 7;
			UEDATX = 4;UEDATX = 5;UEDATX = 6;UEDATX = 7;
			usb_send_in();
			SPIPutChar( 'Y' );
			SPIPutChar( '\n' );
			goto end;
		}
		else if( bRequest == 0xA6 ) //Jumbo  Us to CPU Frame
		{
			unsigned short len;
			SPIPutChar( 'y' );
			SPIPutChar( '\n' );


			//_delay_ms(2);
			//_delay_ms(500);
			len = 128;
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) goto end;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--)
				{
					UEDATX = frame++;
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			//			triggerep = 1;
			goto end;
		}
		else if( bRequest == 0xA5 ) //Control-Out (CPU To Us)  (Only works on control-type messages, else we crash.)
		{
			unsigned i, l;
			unsigned char j;
			unsigned char isPow2 = 0;
			usb_wait_receive_out();
//			SPIPutChar( 'Q' );
			unsigned char c;// = UEDATX;
//			volatile unsigned char v;
			volatile register uint8_t v asm("r3");
			volatile register uint8_t mask asm("r2");

			DDRD |= _BV(3);
			PORTD &= ~_BV(3);


			if( ( wLength & (ENDPOINT0_SIZE-1)) == 0 )
			{
				isPow2 = 1;
			}
/*
			for( i = 0; i < 512; i++ )
			{
				j++;
				if( j == 3 ) j = 0;

				if( j == 1 ) c = 255; else c = 0;

				mask = 0x80;
				while( mask )
				{
					if( mask & c )
					{
						PORTD |= _BV(3);  NOOP; NOOP; //250ns
						NOOP; NOOP; NOOP; NOOP; //250ns
						NOOP; NOOP; NOOP; NOOP; //250ns
						PORTD &= ~_BV(3);
					}
					else
					{
						PORTD |= _BV(3);  NOOP; NOOP; //250ns
						PORTD &= ~_BV(3); NOOP; NOOP; //250ns
						NOOP; NOOP; NOOP; NOOP; //250ns
					}
					mask>>=1;
				}

				if( (i & 63) == 0 ) _delay_us(4);
			}
*/

			//wLength = 1537?

//#define EXTRA_HOLD


#define SEND_WS( var ) \
				mask = 0x80; \
				v = var; \
				while( mask ) \
				{ \
					if( mask & v )  \
					{ \
						PORTD |= _BV(3);  mask>>=1;   \
						NOOP; NOOP; NOOP; NOOP; \
						NOOP; NOOP; NOOP; NOOP; \
						PORTD &= ~_BV(3);\
					} \
					else \
					{ \
						PORTD |= _BV(3); NOOP; \
						PORTD &= ~_BV(3); \
						mask>>=1; \
						NOOP; NOOP; NOOP; NOOP; \
					} \
					\
				}


#if ENDPOINT0_SIZE==64
			l = wLength>>6;
			int remainder = wLength & 63;
#else
			l = wLength>>5;
			int remainder = wLength & 31; 
#endif

			for( i = 0; i < l; i++ )
			{
				buf[0] = UEDATX;
				SEND_WS(buf[0]);
				buf[1] = UEDATX;
				buf[2] = UEDATX;
				buf[3] = UEDATX;
				buf[4] = UEDATX;
				SEND_WS(buf[1]); 
				buf[5] = UEDATX;
				buf[6] = UEDATX;
				buf[7] = UEDATX;
				buf[8] = UEDATX; 
				SEND_WS(buf[2]);
				buf[9] = UEDATX;
				buf[10] = UEDATX; 
				buf[11] = UEDATX; 
				buf[12] = UEDATX; 
				SEND_WS(buf[3]);
				buf[13] = UEDATX; 
				buf[14] = UEDATX; 
				buf[15] = UEDATX; 

				buf[16] = UEDATX;
				buf[17] = UEDATX;
				SEND_WS(buf[4]);
				buf[18] = UEDATX;
				buf[19] = UEDATX;
				buf[20] = UEDATX;
				buf[21] = UEDATX;
				SEND_WS(buf[5]);
				buf[22] = UEDATX;
				buf[23] = UEDATX; 
				buf[24] = UEDATX; 
				SEND_WS(buf[6]);
				buf[25] = UEDATX; 
				buf[26] = UEDATX; 
				buf[27] = UEDATX; 
				SEND_WS(buf[7]);
				buf[28] = UEDATX; 
				buf[29] = UEDATX; 
				buf[30] = UEDATX; 
				buf[31] = UEDATX; 

				SEND_WS(buf[8]);
#if ENDPOINT0_SIZE==64

				buf[32] = UEDATX;
				buf[33] = UEDATX;
				buf[34] = UEDATX;
				buf[35] = UEDATX;
				SEND_WS(buf[9]);

				buf[36] = UEDATX;
				buf[37] = UEDATX;
				buf[38] = UEDATX;
				buf[39] = UEDATX;
				SEND_WS(buf[10]);
 
				buf[40] = UEDATX; 
				buf[41] = UEDATX; 
				buf[42] = UEDATX; 
				buf[43] = UEDATX; 
				SEND_WS(buf[11]);
				buf[44] = UEDATX; 
				buf[45] = UEDATX; 
				buf[46] = UEDATX; 
				buf[47] = UEDATX; 

				SEND_WS(buf[12]);
				buf[48] = UEDATX;
				buf[49] = UEDATX;
				buf[50] = UEDATX;
				buf[51] = UEDATX;
				SEND_WS(buf[13]);
				buf[52] = UEDATX;
				buf[53] = UEDATX;
				buf[54] = UEDATX;
				buf[55] = UEDATX;
				SEND_WS(buf[14]); 
				buf[56] = UEDATX; 
				buf[57] = UEDATX; 
				buf[58] = UEDATX; 
				buf[59] = UEDATX; 
				SEND_WS(buf[15]);
				buf[60] = UEDATX; 
				buf[61] = UEDATX; 
				buf[62] = UEDATX; 
				buf[63] = UEDATX; 
#endif
				if( l-i > 1 )
					usb_ack_out();

#if ENDPOINT0_SIZE != 64
				SEND_WS(buf[9]);
				SEND_WS(buf[10]);
				SEND_WS(buf[11]);
				SEND_WS(buf[12]);
				SEND_WS(buf[13]);
				SEND_WS(buf[14]);
				SEND_WS(buf[15]);
#endif

/*
				SEND_WS(buf[16]);
				SEND_WS(buf[17]);
				SEND_WS(buf[18]);
				SEND_WS(buf[19]);
				SEND_WS(buf[20]);
				SEND_WS(buf[21]);
				SEND_WS(buf[22]);
				SEND_WS(buf[23]);
				SEND_WS(buf[24]);
				SEND_WS(buf[25]);
				SEND_WS(buf[26]);
				SEND_WS(buf[27]);
				SEND_WS(buf[28]);
				SEND_WS(buf[29]);
				SEND_WS(buf[30]);
#if ENDPOINT0_SIZE == 64
				SEND_WS(buf[31]);

				for( j = 32; j < 63; j++ )
				{
					SEND_WS( (buf[j]) );
				}
#endif
*/


#if ENDPOINT0_SIZE == 64
				for( j = 16; j < 63; j++ )
				{
					SEND_WS( (buf[j]) );
#ifdef EXTRA_HOLD
					_delay_us(4);
#endif
				}
#else
				for( j = 16; j < 31; j++ )
				{
					SEND_WS( (buf[j]) );
#ifdef EXTRA_HOLD
					_delay_us(4);
#endif
				}
#endif
				if( l - i > 1 )
					usb_wait_receive_out();

#if ENDPOINT0_SIZE == 64
				SEND_WS(buf[63]);
#else
				SEND_WS(buf[31]);
#endif
			}

			if( remainder )
			{
				usb_ack_out();
				usb_wait_receive_out();
			}

			PORTD &= ~_BV(3);

//			_delay_us(50); //force reset.

			for( i = 0; i < remainder; i++ )
				v = UEDATX;  //read extra byte?

			usb_ack_out();
			usb_send_in();
			goto end;
		}
		else
		{
			//printf( "UNKREQ: %02X\n", bRequest );
			SPIPutChar( '?' );
			SPIPutChar( 'R' );
			sendhex2( bmRequestType );
			sendhex2( bRequest );
			sendhex4( wValue );
			sendhex4( wIndex );
			sendhex4( wLength );
			SPIPutChar( '\n' );
			sendhex2( bRequest );
		}
	}
	UECONX = (1<<STALLRQ) | (1<<EPEN);	// stall

end:
	;
//	if( redo_usb ) goto restart_usb;
//	doing_usb = 0;
//    UDIEN = (1<<EORSTE)|(1<<SOFE);

}


/* 
 * Copyright 2011-2014 Charles Lohr
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
