#ifndef _HAL_APF_H
#define _HAL_APF_H


#include "types.h"


// apf.ccp1sel
#define CCP1SEL_RA2				0b0
#define CCP1SEL_RA5				0b1
// apf.p1bsel
#define P1BSEL_RA0				0b0
#define P1BSEL_RA4				0b1
// apf.txcksel
#define TXCKSEL_RA0				0b0
#define TXCKSEL_RA4				0b1
// apf.t1gsel
#define T1GSEL_RA4				0b0
#define T1GSEL_RA3				0b1
// apf.sssel
#define SSSEL_RA3				0b0
#define SSSEL_RA0				0b1
// apf.sdosel
#define SDOSEL_RA0				0b0
#define SDOSEL_RA4				0b1
// apf.rxdtsel
#define RXDTSEL_RA1				0b0
#define RXDTSEL_RA5				0b1
// apf.con - all settings
#define APFCON_CCP1_RA2			(0b0 << 0)
#define APFCON_CCP1_RA5			(0b1 << 0)
#define APFCON_P1B_RA0			(0b0 << 1)
#define APFCON_P1B_RA4			(0b1 << 1)
#define APFCON_TXCK_RA0			(0b0 << 2)
#define APFCON_TXCK_RA4			(0b1 << 2)
#define APFCON_T1G_RA4			(0b0 << 3)
#define APFCON_T1G_RA3			(0b1 << 3)
#define APFCON_SS_RA3			(0b0 << 5)
#define APFCON_SS_RA0			(0b1 << 5)
#define APFCON_SDO_RA0			(0b0 << 6)
#define APFCON_SDO_RA4			(0b1 << 6)
#define APFCON_RXDT_RA1			(0b0 << 7)
#define APFCON_RXDT_RA5			(0b1 << 7)


typedef union
{
	// APFCON
	struct
	{
		RW uint8_t ccp1sel : 1; // CCP1SEL Pin Selection bit
		RW uint8_t p1bsel  : 1; // P1BSEL Pin Selection bit
		RW uint8_t txcksel : 1; // TXCKSEL Pin Selection bit
		RW uint8_t t1gsel  : 1; // T1GSEL Pin Selection bit
		RO unused_t        : 1; // Unimplemented: Read as '0'
		RW uint8_t sssel   : 1; // SSSEL Pin Selection bit
		RW uint8_t sdosel  : 1; // SDOSEL Pin Selection bit
		RW uint8_t rxdtsel : 1; // RXDTSEL Pin Selection bit
	};
	RW uint8_t con;
} apf_t;


SFR apf_t apf @ 0x11D;


#endif