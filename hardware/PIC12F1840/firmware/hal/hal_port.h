#ifndef _HAL_PORT_H
#define _HAL_PORT_H


#include "types.h"


typedef union
{
	// PORTA
	struct
	{
		RO logic_t i0 : 1;
		RO logic_t i1 : 1;
		RO logic_t i2 : 1;
		RO logic_t i3 : 1;
		RO logic_t i4 : 1;
		RO logic_t i5 : 1;
		RO unused_t   : 2;
	};
	RO uint8_t in;
} pin_t;

typedef union
{
	// PORTA
	struct
	{
		RW logic_t o0 : 1;
		RW logic_t o1 : 1;
		RW logic_t o2 : 1;
		RW logic_t o3 : 1;
		RW logic_t o4 : 1;
		RW logic_t o5 : 1;
		RO unused_t   : 2;
	};
	RW uint8_t out;
} pout_t;

typedef union
{
	// TRISA
	struct
	{
		RW dir_t d0 : 1;
		RW dir_t d1 : 1;
		RW dir_t d2 : 1;
		RW dir_t d3 : 1;
		RW dir_t d4 : 1;
		RW dir_t d5 : 1;
		RO unused_t : 2;
	};
	RW uint8_t dir;
} pdir_t;

typedef union
{
	// LATA
	struct
	{
		RW logic_t l0 : 1;
		RW logic_t l1 : 1;
		RW logic_t l2 : 1;
		RW logic_t l3 : 1;
		RW logic_t l4 : 1;
		RW logic_t l5 : 1;
		RO unused_t   : 2;
	};
	RW uint8_t latch;
} plat_t;

typedef union
{
	// ANSELA
	struct
	{
		RW bool_t a0 : 1;
		RW bool_t a1 : 1;
		RW bool_t a2 : 1;
		RW bool_t a3 : 1;
		RW bool_t a4 : 1;
		RW bool_t a5 : 1;
		RO unused_t  : 2;
	};
	RW uint8_t analog;
} pana_t;

typedef union
{
	// WPUA
	struct
	{
		RW bool_t p0 : 1;
		RW bool_t p1 : 1;
		RW bool_t p2 : 1;
		RW bool_t p3 : 1;
		RW bool_t p4 : 1;
		RW bool_t p5 : 1;
		RO unused_t  : 2;
	};
	RW uint8_t pullup;
} ppul_t;


SFR pin_t  pia @ 0x00C;
SFR pout_t poa @ 0x00C;
SFR pdir_t pda @ 0x08C;
SFR plat_t pla @ 0x10C;
SFR pana_t paa @ 0x18C;
SFR ppul_t ppa @ 0x20C;


#endif