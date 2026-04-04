#ifndef _HAL_INT_H
#define _HAL_INT_H


#include "types.h"
#include "sfr.h"


typedef union
{
	// EIFR
	struct
	{
		RW bool_t int0  : 1; // External Interrupt Flag 0
		RW bool_t int1  : 1; // External Interrupt Flag 1
		RO unused_t     : 6; // Unimplemented: Read as '0'
	};
	RW uint8_t flags;
} eifr_t;

typedef union
{
	// EIMSK
	struct
	{
		RW bool_t int0 : 1; // External Interrupt Request 0 Enable
		RW bool_t int1 : 1; // External Interrupt Request 1 Enable
		RO unused_t    : 6; // Unimplemented: Read as '0'
	};
	RW uint8_t masks;
} eimsk_t;

typedef enum
{
	ISC_LOW    = 0b00,
	ISC_CHANGE = 0b01,
	ISC_FALL   = 0b10,
	ISC_RISE   = 0b11
} eiedge_t;

typedef union
{
	// EICRA
	struct
	{
		RW eiedge_t isc0 : 2; // Interrupt Sense Control 0
		RW eiedge_t isc1 : 2; // Interrupt Sense Control 1
		RO unused_t      : 4; // Unimplemented: Read as '0'
	};
	RW uint8_t edges;
} eicra_t;


SFR_REG(0x1C) eifr_t  eifr;
SFR_REG(0x1D) eimsk_t eimsk;
SFR_REG(0x49) eicra_t eicra;


#endif