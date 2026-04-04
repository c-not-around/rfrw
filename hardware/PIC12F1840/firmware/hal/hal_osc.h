#ifndef _HAL_OSC_H
#define _HAL_OSC_H


#include "types.h"


// osccon.scs - system clock source select
#define SCS_FOSC2					0b00
#define SCS_TIMER1					0b01
#define SCS_INT_OSC					0b10
// osccon.ircf - system clock frequency select
#define IRCF_31K00HZ_LF				0b0000
#define IRCF_31K25HZ_MF				0b0010
#define IRCF_31K25HZ_HF				0b0011
#define IRCF_62K5HZ_MF				0b0100
#define IRCF_125KHZ_MF				0b0101
#define IRCF_250KHZ_MF				0b0110
#define IRCF_500KHZ_MF				0b0111
#define IRCF_125KHZ_HF				0b1000
#define IRCF_250KHZ_HF				0b1001
#define IRCF_500KHZ_HF				0b1010
#define IRCF_1MHZ_HF				0b1011
#define IRCF_2MHZ_HF				0b1100
#define IRCF_4MHZ_HF				0b1101
#define IRCF_8_OR_32MHZ_HF			0b1110
#define IRCF_16MHZ_HF				0b1111
// osc.con - all settings
#define OSCCON_SCS_FOSC2			(0b00   << 0)
#define OSCCON_SCS_TIMER1			(0b01   << 0)
#define OSCCON_SCS_INT_OSC			(0b10   << 0)
#define OSCCON_IRCF_31K00HZ_LF		(0b0000 << 3)
#define OSCCON_IRCF_31K25HZ_MF		(0b0010 << 3)
#define OSCCON_IRCF_31K25HZ_HF		(0b0011 << 3)
#define OSCCON_IRCF_62K5HZ_MF		(0b0100 << 3)
#define OSCCON_IRCF_125KHZ_MF		(0b0101 << 3)
#define OSCCON_IRCF_250KHZ_MF		(0b0110 << 3)
#define OSCCON_IRCF_500KHZ_MF		(0b0111 << 3)
#define OSCCON_IRCF_125KHZ_HF		(0b1000 << 3)
#define OSCCON_IRCF_250KHZ_HF		(0b1001 << 3)
#define OSCCON_IRCF_500KHZ_HF		(0b1010 << 3)
#define OSCCON_IRCF_1MHZ_HF			(0b1011 << 3)
#define OSCCON_IRCF_2MHZ_HF			(0b1100 << 3)
#define OSCCON_IRCF_4MHZ_HF			(0b1101 << 3)
#define OSCCON_IRCF_8_OR_32MHZ_HF	(0b1110 << 3)
#define OSCCON_IRCF_16MHZ_HF		(0b1111 << 3)
#define OSCCON_PLL_DISABLE			(0b0    << 7)
#define OSCCON_PLL_ENABLE			(0b1    << 7)


typedef struct
{
	// OSCTUNE
	RW uint8_t tune; // Frequency Tuning bits [5:0] 0x1F=maximum, 0x40=minimum
	// OSCCON
	union
	{
		struct
		{
			RW uint8_t scs   : 2; // System Clock Select bits
			RO unused_t      : 1; // Unimplemented: Read as '0'
			RW uint8_t ircf  : 4; // Internal Oscillator Frequency Select bits
			RW bool_t  pllen : 1; // Software PLL Enable bit
		} ;
		RW uint8_t con;
	};
} osc_t;


SFR osc_t osc @ 0x098;


#endif