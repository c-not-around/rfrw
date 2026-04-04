#ifndef _HAL_CCP_H
#define _HAL_CCP_H


#include "types.h"


// ccp1.ccp1m - ECCP1 Mode Select bits
#define CCP1M_OFF				0b0000
#define CCP1M_CMP_TOGGLE_OUT	0b0010
#define CCP1M_CAPT_FALL			0b0100
#define CCP1M_CAPT_RISE			0b0101
#define CCP1M_CAPT_4TH_RISE		0b0110
#define CCP1M_CAPT_16TH_RISE	0b0111
#define CCP1M_CMP_SET_OUT		0b1000
#define CCP1M_CMP_CLR_OUT		0b1001
#define CCP1M_CMP_INT_ONLY		0b1010
#define CCP1M_CMP_SPECIAL_TRIG	0b1011
#define CCP1M_PWM_P1AH_P1BH		0b1100
#define CCP1M_PWM_P1AH_P1BL		0b1101
#define CCP1M_PWM_P1AL_P1BH		0b1110
#define CCP1M_PWM_P1AL_P1BL		0b1111
// ccp1.p1m - Enhanced PWM Output Configuration bits
#define P1M_SINGLE_OUTPUT		0b00
#define P1M_HALF_BRIDGE_OUTPUT	0b10


typedef struct
{
	// CCPR1L
	// CCPR1H
	union
	{
		RW uint16_t ccpr1; // Capture/Compare/PWM Register 1
		struct
		{
			RW uint8_t ccpr1l;
			RW uint8_t ccpr1h;
		};
	};
	// CCP1CON
	union
	{
		struct
		{
			RW uint8_t ccp1m : 4; // ECCP1 Mode Select bits
			RW uint8_t dc1b  : 2; // These bits are the two LSbs of the PWM duty cycle. The eight MSbs are found in CCPR1L.
			RW uint8_t p1m   : 2; // Enhanced PWM Output Configuration bits
		};
		RW uint8_t con;
	};
} ccp_t;


SFR ccp_t ccp1 @ 0x291;


#endif