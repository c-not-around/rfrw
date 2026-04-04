#ifndef _HAL_CPS_H
#define _HAL_CPS_H


#include "types.h"


// cps.con0.toxcs
#define T0XCS_T0CKI_PIN			0b0
#define T0XCS_CPSC_OSC			0b1
// cps.con0.rng
#define RNG_OFF					0b00
#define RNG_LOW_RANGE			0b01
#define RNG_MID_RANGE			0b10
#define RNG_HIGH_RANGE			0b11
// cps.con0.rm
#define RM_LOW_RANGE			0b0
#define RM_HIGH_RANGE			0b1
// cps.con0 - all settings
#define CPSCON0_T0CKI_PIN		(0b0  << 0)
#define CPSCON0_CPS_OSC			(0b1  << 0)
#define CPSCON0_RNG_OFF			(0b00 << 2)
#define CPSCON0_RNG_LOW			(0b01 << 2)
#define CPSCON0_RNG_MID			(0b10 << 2)
#define CPSCON0_RNG_HIGH		(0b11 << 2)
#define CPSCON0_RM_LOW			(0b0  << 6)
#define CPSCON0_RM_HIGH			(0b1  << 6)
#define CPSCON0_CPS_OFF			(0b0  << 7)
#define CPSCON0_CPS_ON			(0b1  << 7)

// cps.con1.ch
#define CH_CHANNEL0_CPS0		0b00
#define CH_CHANNEL1_CPS1		0b01
#define CH_CHANNEL2_CPS2		0b10
#define CH_CHANNEL3_CPS3		0b11
// cps.con1 - all settings
#define CPSCON1_CHANNEL0_CPS0	(0b00 << 0)
#define CPSCON1_CHANNEL1_CPS1	(0b01 << 0)
#define CPSCON1_CHANNEL2_CPS2	(0b10 << 0)
#define CPSCON1_CHANNEL3_CPS3	(0b11 << 0)


typedef struct
{
	// CPSCON0
	union
	{
		struct
		{
			RW uint8_t t0xcs : 1; // Timer0 External Clock Source Select bit
			RO logic_t out   : 1; // Capacitive Sensing Oscillator Status bit
			RW uint8_t rng   : 2; // Capacitive Sensing Current Range bit
			RO unused_t      : 2; // Unimplemented: Read as '0'
			RW uint8_t rm    : 1; // Capacitive Sensing Reference Mode bit
			RW bool_t  on    : 1; // CPS Module Enable bit
		};
		RW uint8_t con0;
	};
	// CPSCON1
	union
	{
		struct
		{
			RW uint8_t ch : 2; // Capacitive Sensing Channel Select bits
			RO unused_t   : 6; // Unimplemented: Read as '0'
		};
		RW uint8_t con1;
	};
} cps_t;


SFR cps_t cps @ 0x01E;


#endif