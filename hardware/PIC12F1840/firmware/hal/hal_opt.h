#ifndef _HAL_OPT_H
#define _HAL_OPT_H


#include "types.h"


// opt.ps - prescaller division coefficient
#define PS_COEF_2				0b000
#define PS_COEF_4				0b001
#define PS_COEF_8				0b010
#define PS_COEF_16				0b011
#define PS_COEF_32				0b100
#define PS_COEF_64				0b101
#define PS_COEF_128				0b110
#define PS_COEF_256				0b111
// opt.psa - prescaller module recipient
#define PSA_TMR0				0b0
#define PSA_WDT					0b1
// tmr0se - timer0 increment edge (ext clock source)
#define TMR0SE_RISE				0b0
#define TMR0SE_FALL				0b1
// tmr0cs - timer0 clock source select
#define TMR0CS_INTERNAL			0b0
#define TMR0CS_EXTERNAL			0b1
// opt.con - all settings
#define OPTCON_PS_COEF_2		(0b000 << 0)
#define OPTCON_PS_COEF_4		(0b001 << 0)
#define OPTCON_PS_COEF_8		(0b010 << 0)
#define OPTCON_PS_COEF_16		(0b011 << 0)
#define OPTCON_PS_COEF_32		(0b100 << 0)
#define OPTCON_PS_COEF_64		(0b101 << 0)
#define OPTCON_PS_COEF_128		(0b110 << 0)
#define OPTCON_PS_COEF_256		(0b111 << 0)
#define OPTCON_PSA_TMR0			(0b0   << 3)
#define OPTCON_PSA_WDT			(0b1   << 3)
#define OPTCON_TMR0_SE_RISE		(0b0   << 4)
#define OPTCON_TMR0_SE_FALL		(0b1   << 4)
#define OPTCON_TMR0_CS_INT		(0b0   << 5)
#define OPTCON_TMR0_CS_EXT		(0b1   << 5)
#define OPTCON_INTEDG_FALL		(0b0   << 6)
#define OPTCON_INTEDG_RISE		(0b1   << 6)
#define OPTCON_WPU_ENABLE		(0b0   << 7)
#define OPTCON_WPU_DISABLE		(0b1   << 7)


typedef union
{
	// OPTION_REG
	struct
	{
		RW uint8_t ps     : 3; // Prescaler Rate Select bits
		RW uint8_t psa    : 1; // Prescaler Assignment bit
		RW uint8_t tmr0se : 1; // Timer0 Source Edge Select bit
		RW uint8_t tmr0cs : 1; // Timer0 Clock Source Select bit
		RW edge_t  intedg : 1; // Interrupt Edge Select bit
		RW bool_t  wpudis : 1; // Weak Pull-up Enable bit
	};
	RW uint8_t con;
} opt_t;


SFR opt_t opt @ 0x095;


#endif