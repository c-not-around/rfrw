#ifndef _HAL_PORT_H
#define _HAL_PORT_H


#include "types.h"
#include "sfr.h"


typedef union
{
	struct
	{
		// PINx
		RO logic_t i0 : 1;
		RO logic_t i1 : 1;
		RO logic_t i2 : 1;
		RO logic_t i3 : 1;
		RO logic_t i4 : 1;
		RO logic_t i5 : 1;
		RO logic_t i6 : 1;
		RO logic_t i7 : 1;
		// DDRx
		RW dir_t   d0 : 1;
		RW dir_t   d1 : 1;
		RW dir_t   d2 : 1;
		RW dir_t   d3 : 1;
		RW dir_t   d4 : 1;
		RW dir_t   d5 : 1;
		RW dir_t   d6 : 1;
		RW dir_t   d7 : 1;
		// PORTx
		RW logic_t o0 : 1;
		RW logic_t o1 : 1;
		RW logic_t o2 : 1;
		RW logic_t o3 : 1;
		RW logic_t o4 : 1;
		RW logic_t o5 : 1;
		RW logic_t o6 : 1;
		RW logic_t o7 : 1;
	};
	struct
	{
		RW uint8_t in;  // PINx
		RW uint8_t dir; // DDRx
		RW uint8_t out; // PORTx
	};
} port_t;

typedef struct
{
	port_t b;
	port_t c;
	port_t d;
} ports_t;


SFR_REG(0x03) ports_t ports;


#endif