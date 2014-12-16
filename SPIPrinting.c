#include "SPIPrinting.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#ifndef NO_PRINTF

volatile char SPIBuffer[PRINTF_BUFFER_SIZE];
volatile unsigned char SPIHead = 0;
volatile unsigned char SPITail = 0;

static int SPIPutCharInternal(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM( SPIPutCharInternal, NULL, _FDEV_SETUP_WRITE );

void SetupPrintf()
{
	SPCR = (1<<SPIE)|(1<<SPE);
	DDRB |= _BV(3);
	DDRB &= ~(_BV(0)|_BV(1)|_BV(2));
	stdout = &mystdout;
}

static int SPIPutCharInternal(char c, FILE *stream)
{
	if( ( SPIHead + 1 ) == SPITail ) return -1;//Overflow.
	SPIBuffer[SPIHead] = c;
	SPIHead++;
	if( SPIHead == PRINTF_BUFFER_SIZE ) SPIHead = 0;
	return 0;
}

void SPIPutChar( char c )
{
	SPIPutCharInternal( c, 0 );
}

void SPISendStr( const char * cs )
{
	char c;
	while( c = *(cs++) )
		SPIPutChar( c );
}


ISR( SPI_STC_vect )
{
//	volatile unsigned char cx = SPDR;
//	If you want to get input from the computer, read SPDR.

	if( SPITail != SPIHead )
		SPDR = SPIBuffer[SPITail++];
	else
		SPDR = 0;

	if( SPITail == PRINTF_BUFFER_SIZE )
		SPITail = 0;
}


static void sendhex1( unsigned char val )
{
	SPIPutChar(((val)>9)?((val)+'a'-10):((val)+'0'));
}

void sendhex2( unsigned char val )
{
	sendhex1( val >> 4 );
	sendhex1( val & 0x0f );
}

void sendhex3( unsigned short val )
{
	sendhex1( ( val >> 8 ) & 0x0f );
	sendhex1( ( val >> 4 ) & 0x0f );
	sendhex1( val & 0x0f );
}

void sendhex4( unsigned short val )
{
	sendhex1( ( val >> 12 ) & 0x0f );
	sendhex1( ( val >> 8 ) & 0x0f );
	sendhex1( ( val >> 4 ) & 0x0f );
	sendhex1( val & 0x0f );
}

void senddec2( unsigned char val )
{
	unsigned char v10 = val/10;
	unsigned char lo = val%10;
	if( v10 > 9 )
		SPIPutChar( '+' );
	else
		SPIPutChar( '0' + v10 );
	SPIPutChar( '0' + lo );
}
#endif
