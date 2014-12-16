#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

extern "C" 
{
#include <libusb-1.0/libusb.h>
};

struct libusb_device_handle *devh = NULL;

int doexit = 0;

void sigexit( int sig )
{
	doexit = 1;
}

unsigned char hue( unsigned char x )
{
	if( x < 42 )
	{
		unsigned short ret = x << 2;
		ret += x << 1;
		if( ret > 255 ) ret = 255;
		return ret;
	}
	else if( x < 128 )
	{
		return 255;
	}
	else if( x < 170 )
	{
		unsigned short ret = x - 128;
		return 255 - ret*6;
/*		short ret = (x-128) << 2;
		ret += (x-128) << 1;
		if( ret > 255 ) ret = 255;
		if( ret < 0 ) ret = 0;
		return 255 - x;		*/
	}
	else
	{
		return 0;
	}
}


unsigned long HSVtoHEX( float hue, float sat, float value )
{
	float pr = 0;
	float pg = 0;
	float pb = 0;

	short ora = 0;
	short og = 0;
	short ob = 0;

	float ro = fmod( hue * 6, 6. );

	float avg = 0;

	ro = fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

	if( ro < 1 ) //yellow->red
	{
		pr = 1;
		pg = 1. - ro;
	} else if( ro < 2 )
	{
		pr = 1;
		pb = ro - 1.;
	} else if( ro < 3 )
	{
		pr = 3. - ro;
		pb = 1;
	} else if( ro < 4 )
	{
		pb = 1;
		pg = ro - 3;
	} else if( ro < 5 )
	{
		pb = 5 - ro;
		pg = 1;
	} else
	{
		pg = 1;
		pr = ro - 5;
	}

	//Actually, above math is backwards, oops!
	pr *= value;
	pg *= value;
	pb *= value;

	avg += pr;
	avg += pg;
	avg += pb;

	pr = pr * sat + avg * (1.-sat);
	pg = pg * sat + avg * (1.-sat);
	pb = pb * sat + avg * (1.-sat);

	ora = pr * 255;
	og = pb * 255;
	ob = pg * 255;

	if( ora < 0 ) ora = 0;
	if( ora > 255 ) ora = 255;
	if( og < 0 ) og = 0;
	if( og > 255 ) og = 255;
	if( ob < 0 ) ob = 0;
	if( ob > 255 ) ob = 255;

	return (ob<<16) | (og<<8) | ora;
}


int main()
{
	int frame;
	int r, i;


	if( libusb_init(NULL) < 0 )
	{
		fprintf( stderr, "Error: Could not initialize libUSB\n" );
		return -1;
	}

	devh = libusb_open_device_with_vid_pid( NULL, 0xabcd, 0xf003 );

	if( !devh )
	{
		fprintf( stderr,  "Error: Cannot find device.\n" );
		return -1;
	}
/*
	if( libusb_claim_interface(devh, 0) < 0 )
	{
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return -1;
	}
*/
	printf( "Interface ok!\n" );

	printf( "XActions Done\n" );

	signal( SIGINT, sigexit );
	signal( SIGTERM, sigexit );
	signal( SIGQUIT, sigexit );

	while( !doexit )
	{
		unsigned char rbuffer[65536];

//		if( libusb_handle_events(NULL) < 0 )
//		{
//			doexit = 1;
//			}
		//

//int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
//	uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
//	unsigned char *data, uint16_t wLength, unsigned int timeout);

//#define LEDS 1365
#define LEDS 512
		//memset( rbuffer, 0, sizeof( rbuffer ) );
		for( i = 0; i < LEDS; i++ )
		{
			int r;
			r = HSVtoHEX( (double)(i)/12.7 + ((double)frame)/199.55, 1, 1.0 );
			r = HSVtoHEX( (double)(i)/1200.7 + ((double)frame)/190.55, 1, ( (((i+frame)/1)%600) ) / 600.0  );


//			r &= 0x00ff00;
//			r = 0x101010;
//			r = 0xffffff;
//			r = 0x000000;
//			int rr = frame & 0x1ff;
//			if( rr > 0xff ) rr = 0x1ff - rr;
//			r |= (((255-(rr&0xff)))<<16) | ((rr&0xff));

			rbuffer[i*3+0] = r & 0xff;
			rbuffer[i*3+1] = (r>>8) & 0xff;
			rbuffer[i*3+2] = (r>>16) & 0xff;
		}
#define SENDING

//Read from device
#ifndef SENDING
		int r = libusb_control_transfer( devh,
			0x80, //reqtype
			0xA6, //request
			0x0100, //wValue
			0x0000, //wIndex
			rbuffer,
			8192,
			1000 );
#else
#define DOTTER
		int r = libusb_control_transfer( devh,
			0x40, //reqtype
			0xA5, //request
			0x0100, //wValue
			0x0000, //wIndex
			rbuffer,
			(LEDS*3)+1,
			1000 );
#endif

#ifdef DOTTER
		if( r > 0 )
		{
			printf( "." );
			fflush( stdout );
		}
		else
		{
			fprintf( stderr, "Error: %d\n", r );
		}

#else

		printf( "%d ", r );
		for( i = 0; i < r; i++ )
				printf( " %02x ", rbuffer[i] );
		printf( "\n" );

#endif
		frame++;

		//usleep(50000);
	}

	libusb_release_interface( devh, 0 );
	libusb_close( devh );
	libusb_exit( NULL );
	printf( "Exit ok.\n" );
	return -1;

}

