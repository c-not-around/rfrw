#ifndef _HAL_FVR_H
#define _HAL_FVR_H


#include "types.h"


// fvr.ad
#define AD_OFF					0b00
#define AD_1X_1V024				0b01
#define AD_1X_2V048				0b10
#define AD_1X_4V096				0b11
// fvr.cda
#define CDA_OFF					0b00
#define CDA_1X_1V024			0b01
#define CDA_1X_2V048			0b10
#define CDA_1X_4V096			0b11
// fvr.tsrng
#define TSRNG_HIGH_RANGE		0b0
#define TSRNG_LOW_RANGE			0b1
// fvr.con - all settings
#define FVRCON_AD_OFF			(0b00 << 0)
#define FVRCON_AD_1X_1V024		(0b01 << 0)
#define FVRCON_AD_1X_2V048		(0b10 << 0)
#define FVRCON_AD_1X_4V096		(0b11 << 0)
#define FVRCON_CDA_OFF			(0b00 << 2)
#define FVRCON_CDA_1X_1V024		(0b01 << 2)
#define FVRCON_CDA_1X_2V048		(0b10 << 2)
#define FVRCON_CDA_1X_4V096		(0b11 << 2)
#define FVRCON_HIGH_RANGE		(0b0  << 4)
#define FVRCON_LOW_RANGE		(0b1  << 4)
#define FVRCON_TEMP_IND_OFF		(0b0  << 5)
#define FVRCON_TEMP_IND_ON		(0b1  << 5)
#define FVRCON_FVR_OFF			(0b0  << 7)
#define FVRCON_FVR_ON			(0b1  << 7)

typedef union
{
	// FVRCON
	struct
	{
		RW uint8_t ad    : 2; // ADC Fixed Voltage Reference Selection bits
		RW uint8_t cda   : 2; // Comparator and DAC Fixed Voltage Reference Selection bits
		RW uint8_t tsrng : 1; // Temperature Indicator Range Selection bit
		RW bool_t  tsen  : 1; // Temperature Indicator Enable bit
		RO bool_t  ready : 1; // Fixed Voltage Reference Ready Flag bit
		RW bool_t  en    : 1; // Fixed Voltage Reference Enable bit
 	};
	RW uint8_t con;
} fvr_t;


SFR fvr_t fvr @ 0x117;


#endif