#ifndef _HAL_TIMER_H
#define _HAL_TIMER_H


#include "types.h"


// timer1.ckps
#define T1_CKPS_COEF_1			0b00
#define T1_CKPS_COEF_2			0b01
#define T1_CKPS_COEF_4			0b10
#define T1_CKPS_COEF_8			0b11
// timer1.cs
#define T1_CS_FOSC_DIV_4		0b00
#define T1_CS_FOSC_DIV_1		0b01
#define T1_CS_T1CKI_OR_T1OSI	0b10
#define T1_CS_CAPOSC			0b11
// timer1.con - all settings
#define T1CON_OFF				(0b0  << 0)
#define T1CON_ON				(0b1  << 0)
#define T1CON_SYNC_ON			(0b0  << 2)
#define T1CON_SYNC_OFF			(0b1  << 2)
#define T1CON_OSCEN_OFF			(0b0  << 3)
#define T1CON_OSCEN_ON			(0b1  << 3)
#define T1CON_CKPS_COEF_1		(0b00 << 4)
#define T1CON_CKPS_COEF_2		(0b01 << 4)
#define T1CON_CKPS_COEF_4		(0b10 << 4)
#define T1CON_CKPS_COEF_5		(0b11 << 4)
#define T1CON_CS_FOSC_DIV_4		(0b00 << 6)
#define T1CON_CS_FOSC_DIV_1		(0b01 << 6)
#define T1CON_CS_T1CKI_OR_T1OSI	(0b10 << 6)
#define T1CON_CS_CAPOSC			(0b11 << 6)

// timer2.ckps
#define T2_CKPS_COEF_1			0b00
#define T2_CKPS_COEF_4			0b01
#define T2_CKPS_COEF_16			0b10
#define T2_CKPS_COEF_64			0b11
// timer2.ckps
#define T2_OUTPS_COEF_1			0b0000
#define T2_OUTPS_COEF_2			0b0001
#define T2_OUTPS_COEF_3			0b0010
#define T2_OUTPS_COEF_4			0b0011
#define T2_OUTPS_COEF_5			0b0100
#define T2_OUTPS_COEF_6			0b0101
#define T2_OUTPS_COEF_7			0b0110
#define T2_OUTPS_COEF_8			0b0111
#define T2_OUTPS_COEF_9			0b1000
#define T2_OUTPS_COEF_10		0b1001
#define T2_OUTPS_COEF_11		0b1010
#define T2_OUTPS_COEF_12		0b1011
#define T2_OUTPS_COEF_13		0b1100
#define T2_OUTPS_COEF_14		0b1101
#define T2_OUTPS_COEF_15		0b1110
#define T2_OUTPS_COEF_16		0b1111
// timer2.con - all settings
#define T2CON_CKPS_COEF_1		(0b00   << 0)
#define T2CON_CKPS_COEF_4		(0b01   << 0)
#define T2CON_CKPS_COEF_16		(0b10   << 0)
#define T2CON_CKPS_COEF_64		(0b11   << 0)
#define T2CON_OFF				(0b0    << 2)
#define T2CON_ON				(0b1    << 2)
#define T2CON_OUTPS_COEF_1		(0b0000 << 3)
#define T2CON_OUTPS_COEF_2		(0b0001 << 3)
#define T2CON_OUTPS_COEF_3		(0b0010 << 3)
#define T2CON_OUTPS_COEF_4		(0b0011 << 3)
#define T2CON_OUTPS_COEF_5		(0b0100 << 3)
#define T2CON_OUTPS_COEF_6		(0b0101 << 3)
#define T2CON_OUTPS_COEF_7		(0b0110 << 3)
#define T2CON_OUTPS_COEF_8		(0b0111 << 3)
#define T2CON_OUTPS_COEF_9		(0b1000 << 3)
#define T2CON_OUTPS_COEF_10		(0b1001 << 3)
#define T2CON_OUTPS_COEF_11		(0b1010 << 3)
#define T2CON_OUTPS_COEF_12		(0b1011 << 3)
#define T2CON_OUTPS_COEF_13		(0b1100 << 3)
#define T2CON_OUTPS_COEF_14		(0b1101 << 3)
#define T2CON_OUTPS_COEF_15		(0b1110 << 3)
#define T2CON_OUTPS_COEF_16		(0b1111 << 3)



typedef struct
{
	// TMR1L
	// TMR1H
	union
	{
		RW uint16_t tmr1; // Timer1 Counter Register
		struct
		{
			RW uint8_t tmr1l;
			RW uint8_t tmr1h;
		};
	};
	// T1CON
	union
	{
		struct
		{
			RW bool_t  on    : 1; // Timer1 On bit
			RO unused_t      : 1; // Unimplemented: Read as '0'
			RW bool_t  sync  : 1; // Timer1 External Clock Input Synchronization Control bit
			RW bool_t  oscen : 1; // LP Oscillator Enable Control bit
			RW uint8_t ckps  : 2; // Timer1 Input Clock Prescale Select bits
			RW uint8_t cs    : 2; // Timer1 Clock Source Select bits
		};
		RW uint8_t con;
	};
} timer1_t;

typedef struct
{
	// TMR2
	RW uint8_t tmr2; // Timer2 Counter Register
	// PR2
	RW uint8_t pr2;  // Timer2 Module Period Register
	// T2CON
	union
	{
		struct
		{
			RW uint8_t ckps  : 2; // Timer2 Input Clock Prescale Select bits
			RW bool_t  on    : 1; // Timer2 On bit
			RW uint8_t outps : 4; // Timer2 Output Postscaler Select bits
			RO unused_t      : 1; // Unimplemented: Read as '0'
		};
		RW uint8_t con;
	};
} timer2_t;


SFR timer1_t timer1 @ 0x016;
SFR timer2_t timer2 @ 0x01A;


#endif