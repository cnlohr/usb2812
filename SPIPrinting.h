//Copyright 2011 Charles Lohr under the MIT/X11 License.

#ifndef _PRINTFS_H
#define _PRINTFS_H

#define PRINTF_BUFFER_SIZE 128

#define NO_PRINTF

#ifdef NO_PRINTF

#define SetupPrintf()
#define SPIPutChar(x)
#define sendhex2(x)
#define sendhex3(x)
#define sendhex4(x)
#define senddec2(x)
#define SPISendStr(x)

#else

void SetupPrintf();

void SPIPutChar( char c );
void SPISendStr( const char * cs );
//#define sendhex1( val )  SPIPutChar(((val)>9)?((val)+'a'-10):((val)+'0'))

void sendhex2( unsigned char val );
void sendhex3( unsigned short val );
void sendhex4( unsigned short val );
void senddec2( unsigned char val );

#endif

#endif

