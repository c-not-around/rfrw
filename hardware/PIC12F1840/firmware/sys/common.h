#ifndef _COMMON_H
#define _COMMON_H


#include <pic.h>
#include "types.h"


#define KHZ					1000UL
#define MHZ					1000000UL

#ifndef _XTAL_FREQ
#define _XTAL_FREQ			(32*MHZ)
#endif

#define COMMAND_CYCLE		4

#define BYTE_LSB			(1<<0)
#define BYTE_MSB			(1<<7)

#define NIBBLE_BITS			4
#define NIBBLE_LOW			0b1111
#define NIBBLE_HIGH			(NIBBLE_LOW<<NIBBLE_BITS)

#define LOW_BYTE(x)			(*((uint8_t*)&(x)+0))
#define HIGH_BYTE(x)		(*((uint8_t*)&(x)+1))
#define LOW_WORD(x)			(*((uint16_t*)&(x)+0))
#define HIGH_WORD(x)		(*((uint16_t*)&(x)+1))
#define BYTE(x,n)			(*((uint8_t*)&(x)+(n)))

#define HEX(x)				(((x)<10?'0':'7')+(x))

#define MAIN()				void main()
#define INIT()				
#define MAIN_LOOP()			while (1)


#endif