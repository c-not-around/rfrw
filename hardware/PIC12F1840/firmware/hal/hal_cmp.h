#ifndef _HAL_CMP_H
#define _HAL_CMP_H


#include "types.h"


// cmp.con0 - all settings
#define CMPCON0_CMP_OUT_ASYNC	(0b0 << 0)
#define CMPCON0_CMP_OUT_SYNC	(0b1 << 0)
#define CMPCON0_HYST_OFF		(0b0 << 1)
#define CMPCON0_HYST_ON			(0b1 << 1)
#define CMPCON0_SPEED_LOW		(0b0 << 2)
#define CMPCON0_SPEED_HIGH		(0b1 << 2)
#define CMPCON0_POL_DIR			(0b0 << 4)
#define CMPCON0_POL_INV			(0b1 << 4)
#define CMPCON0_OUT_DISABLE		(0b0 << 5)
#define CMPCON0_OUT_ENABLE		(0b1 << 5)
#define CMPCON0_CMP_OFF			(0b0 << 7)
#define CMPCON0_CMP_ON			(0b1 << 7)

// cmp.con1.nch
#define NCH_C1IN0M_PIN			0b0
#define NCH_C1IN1M_PIN			0b1
// cmp.con1.pch
#define PCH_C1INP_PIN			0b00
#define PCH_DAC_VREF_PIN		0b01
#define PCH_FVR_VREF_PIN		0b10
// cmp.con1 - all settings
#define CMPCON1_C1IN0M_PIN		(0b0  << 0)
#define CMPCON1_C1IN1M_PIN		(0b1  << 0)
#define CMPCON1_C1INP_PIN		(0b00 << 4)
#define CMPCON1_DAC_VREF_PIN	(0b01 << 4)
#define CMPCON1_FVR_VREF_PIN	(0b10 << 4)
#define CMPCON1_INTN_DISABLE	(0b0  << 6)
#define CMPCON1_INTN_ENABLE		(0b1  << 6)
#define CMPCON1_INTP_DISABLE	(0b0  << 7)
#define CMPCON1_INTP_ENABLE		(0b1  << 7)


typedef struct
{
	// CM1CON0
	union
	{
		struct
		{
			RW bool_t sync : 1; // Comparator Output Synchronous Mode bit
			RW bool_t hyst : 1; // Comparator Hysteresis Enable bit
			RW bool_t sp   : 1; // Comparator Speed/Power Select bit
			RO unused_t    : 1; // Unimplemented: Read as '0'
			RW bool_t pol  : 1; // Comparator Output Polarity Select bit
			RW bool_t oe   : 1; // Comparator Output Enable bit
			RO bool_t out  : 1; // Comparator Output bit
			RW bool_t on   : 1; // Comparator Enable bit
		};
		RW uint8_t con0;
	};
	// CM1CON1
	union
	{
		struct
		{
			RW uint8_t nch  : 1; // Comparator Negative Input Channel Select bit
			RO unused_t     : 3; // Unimplemented: Read as '0'
			RW uint8_t pch  : 2; // Comparator Positive Input Channel Select bits
			RW bool_t  intn : 1; // Comparator Interrupt on Negative Going Edge Enable bits
			RW bool_t  intp : 1; // Comparator Interrupt on Positive Going Edge Enable bits
		};
		RW uint8_t con1;
	};
} cm1_t;


SFR cm1_t cm1 @ 0x111;


#endif