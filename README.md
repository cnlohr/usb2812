ATMega32u2 USB WS2812B Driver
=============================

A number of people often comment about the sizes of USB messages, and the need to buffer things.  That is ridiculous.  This particularly bothers me when it comes to WS2812's.  Though they have not-so-lenient timing requirements, such as demanding that all data is sent in one go... It is possible to concurrently do things on an AVR while sending data to the WS2812.

You can actually cause USB messages to continue across multiple messages, interleved with sending to the WS2812.  That allows a single control message to be significantly larger than the endpoint size - and you can reap excellent performance increases and low-latency!

For this project, it is a WS2812 driver for a ATMega32u2.  I've tested it up to 1365 LEDs per AVR.  The AVR only has 1kB of RAM.  This is a 4kB operation.  Turns out you don't need to buffer any of this stuff.

How to talk to the AVR?
-----------------------

It's particularly easy to use.  In libUSB, connect, then send it 0xA5 messages.
```c
	libusb_init(NULL)
	devh = libusb_open_device_with_vid_pid( NULL, 0xabcd, 0xf003 );
	libusb_control_transfer( devh,
		0x40, //reqtype
		0xA5, //request
		0x0100, //wValue
		0x0000, //wIndex
		rbuffer,    //LED Data
		(LEDS*3)+1, //# of bytes for LEDs
		1000 );     //Timeout
```

On the AVR
----------

The idea behind how this works is as follows (see usb.c)

* Put code in the AVR's ISR(USB_COM_vect).
* Compute how many blocks will be needed.
* Compute how much remainder there will be (if bytes transferred does not fill frame)
* Start reading data; interleving the WS2812 outputs.
```c
...
	buf[0] = UEDATX;  //Read data from USB
	SEND_WS(buf[0]);  //Send WS2812 Data.
	buf[1] = UEDATX;  //Read a bunch more data.
	buf[2] = UEDATX;
	buf[3] = UEDATX;
	buf[4] = UEDATX;  //Can't wait too long before sending another WS2812 byte.
	SEND_WS(buf[1]); 
	buf[5] = UEDATX;
	buf[6] = UEDATX;
	buf[7] = UEDATX;
	buf[8] = UEDATX; 
	SEND_WS(buf[2]);
	buf[9] = UEDATX;
	buf[10] = UEDATX; 
...
	buf[59] = UEDATX; //We were able to sneak a lot of data out, while sending to the WS2812.
	SEND_WS(buf[15]);
	buf[60] = UEDATX; 
	buf[61] = UEDATX; 
	buf[62] = UEDATX; 
	buf[63] = UEDATX; 

	//In fact, a whole buffer's worth, we can ack it so the data keeps flowing.

	if( l-i > 1 )
		usb_ack_out();

	//While the USB hardware is handling getting another buffer in, we can flush out the WS2812.
	for( j = 16; j < 63; j++ )
	{
		SEND_WS( (buf[j]) );
		_delay_us(4);
	}

	//That took a long time, so by now, the data should be ready to read!
	if( l - i > 1 )
		usb_wait_receive_out();

(Repeat)
```

License Info
------------

```
 * USB Keyboard Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * Copyright (c) 2011-2014 <>< Charles Lohr
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
```

