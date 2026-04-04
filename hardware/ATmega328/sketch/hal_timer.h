#ifndef _HAL_TIMER_H
#define _HAL_TIMER_H


#include "types.h"
#include "sfr.h"


typedef	enum
{
	T_TO_MAX_L       = 0b00,
	T_PWM_PC_MAX_L   = 0b01,
	T_TO_OCR_L       = 0b10,
	T_PWM_FAST_MAX_L = 0b11,
	T_PWM_PC_OCR_L   = 0b01,
	T_PWM_FAST_OCR_L = 0b11
} twgml_t;

typedef	enum
{
	T_TO_MAX_H       = 0b0,
	T_PWM_PC_MAX_H   = 0b0,
	T_TO_OCR_H       = 0b0,
	T_PWM_FAST_MAX_H = 0b0,
	T_PWM_PC_OCR_H   = 0b1,
	T_PWM_FAST_OCR_H = 0b1
} twgmh_t;

typedef enum
{
	T1_TO_MAX_L           = 0b00,
	T1_PWM_PC_OCR_8BIT_L  = 0b01,
	T1_PWM_PC_OCR_9BIT_L  = 0b10,
	T1_PWM_PC_OCR_10BIT_L = 0b11,
	T1_TO_OCR_L           = 0b00,
	T1_PWM_FAST_8BIT_L    = 0b01,
	T1_PWM_FAST_9BIT_L    = 0b10,
	T1_PWM_FAST_10BIT_L   = 0b11,
	T1_PWM_PFC_ICR_L      = 0b00,
	T1_PWM_PFC_OCR_L      = 0b01,
	T1_PWM_PC_ICR_L       = 0b10,
	T1_PWM_PC_OCR_L       = 0b11,
	T1_TO_ICR_L           = 0b00,
	T1_PWM_FAST_ICR_L     = 0b10,
	T1_PWM_FAST_OCR_L     = 0b11
} t1wgml_t;

typedef enum
{
	T1_TO_MAX_H           = 0b00,
	T1_PWM_PC_OCR_8BIT_H  = 0b00,
	T1_PWM_PC_OCR_9BIT_H  = 0b00,
	T1_PWM_PC_OCR_10BIT_H = 0b00,
	T1_TO_OCR_H           = 0b01,
	T1_PWM_FAST_8BIT_H    = 0b01,
	T1_PWM_FAST_9BIT_H    = 0b01,
	T1_PWM_FAST_10BIT_H   = 0b01,
	T1_PWM_PFC_ICR_H      = 0b10,
	T1_PWM_PFC_OCR_H      = 0b10,
	T1_PWM_PC_ICR_H       = 0b10,
	T1_PWM_PC_OCR_H       = 0b10,
	T1_TO_ICR_H           = 0b11,
	T1_PWM_FAST_ICR_H     = 0b11,
	T1_PWM_FAST_OCR_H     = 0b11
} t1wgmh_t;

typedef enum
{
	T_NORMAL = 0b00,
	T_TOGGLE = 0b01,
	T_CLEAR  = 0b10,
	T_SET    = 0b11
} tcmp_t;

typedef enum
{
	T_DISABLE  = 0b000,
	T_DIV_1    = 0b001,
	T_DIV_8    = 0b010,
	T_DIV_64   = 0b011,
	T_DIV_256  = 0b100,
	T_DIV_1024 = 0b101,
	T_EXT_FALL = 0b110,
	T_EXT_RISE = 0b111
} tcs_t;

typedef enum
{
	T2_DISABLE  = 0b000,
	T2_DIV_1    = 0b001,
	T2_DIV_8    = 0b010,
	T2_DIV_32   = 0b011,
	T2_DIV_64   = 0b100,
	T2_DIV_128  = 0b101,
	T2_DIV_256  = 0b110,
	T2_DIV_1024 = 0b111
} t2cs_t;

typedef struct
{
	// TCCR0A
	RW twgml_t wgml   : 2; // Waveform Generation Mode
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW tcmp_t  comb   : 2; // Compare Output Mode for Channel B
	RW tcmp_t  coma   : 2; // Compare Output Mode for Channel A
	//	TCCR0B
	RW tcs_t   cs     : 3; // Clock Select 0
	RW twgmh_t wgmh   : 1; // Waveform Generation Mode
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW bool_t  focb   : 1; // Force Output Compare B
	RW bool_t  foca   : 1; // Force Output Compare A
	// TCNT0
	RW uint8_t tcnt;       // TC0 Counter Value
	// OCR0A
	RW uint8_t ocra;       // Output Compare Register 0 A
	// OCR0B
	RW uint8_t ocrb;       // Output Compare Register 0 B
} timer0_t;

typedef struct
{
	// TCCR2A
	RW twgml_t wgml   : 2; // Waveform Generation Mode
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW tcmp_t  comb   : 2; // Compare Output Mode for Channel B
	RW tcmp_t  coma   : 2; // Compare Output Mode for Channel A
	// TCCR2B
	RW t2cs_t cs      : 3; // Clock Select 2
	RW twgmh_t wgmh   : 1; // Waveform Generation Mode
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW bool_t  focb   : 1; // Force Output Compare B
	RW bool_t  foca   : 1; // Force Output Compare A
	// TCNT2
	RW uint8_t tcnt;       // TC2 Counter Value
	// OCR2A
	RW uint8_t ocra;       // Output Compare Register 2 A
	// OCR2B
	RW uint8_t ocrb;       // Output Compare Register 2 B
	// 0xB5
	RO unused_t       : 8; // Unimplemented: Read as '0'
	// ASSR
	RO bool_t tcrbub  : 1; // Timer/Counter Control Register2B Update Busy
	RO bool_t tcraub  : 1; // Timer/Counter Control Register2A Update Busy
	RO bool_t ocrbub  : 1; // Output Compare Register2B Update Busy
	RO bool_t ocraub  : 1; // Output Compare Register2A Update Busy
	RO bool_t ub      : 1; // Timer/Counter2 Update Busy
	RO bool_t as      : 1; // Asynchronous Timer/Counter2
	RO bool_t exclk   : 1; // Enable External Clock Input
	RO unused_t       : 1; // Unimplemented: Read as '0'
} timer2_t;

typedef struct
{
	// TCCR1A
	RW t1wgml_t wgml  : 2; // Waveform Generation Mode
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW tcmp_t   comb  : 2; // Compare Output Mode for Channel B
	RW tcmp_t   coma  : 2; // Compare Output Mode for Channel A
	// TCCR1B
	RW tcs_t    cs    : 3; // Clock Select
	RW t1wgmh_t wgmh  : 2; // Waveform Generation Mode
	RO unused_t       : 1; // Unimplemented: Read as '0'
	RW edge_t   ices  : 1; // Input Capture Edge Select
	RW bool_t   icnc  : 1; // Input Capture Noise Canceler
	// TCCR1C
	RO unused_t       : 6; // Unimplemented: Read as '0'
	RW bool_t   focb  : 1; // Force Output Compare for Channel B
	RW bool_t   foca  : 1; // Force Output Compare for Channel A
	// 0x83
	RO unused_t       : 8; // Unimplemented: Read as '0'
	// TCNT1L
	RW uint16_t tcnt;      // TC1 Counter Value
	// ICR1L
	RW uint16_t icr;       // Input Capture Register 1
	// OCR1AL
	RW uint16_t ocra;      // Output Compare Register 1 A
	// OCR1BL
	RW uint16_t ocrb;      // Output Compare Register 1 B
} timer1_t;

typedef struct
{
	// GTCCR
	RW bool_t psrsync : 1; // Prescaler Reset
	RW bool_t psrasy  : 1; // Prescaler Reset Timer/Counter2
	RO unused_t       : 5; // Unimplemented: Read as '0'
	RW bool_t tsm     : 1; // Timer/Counter Synchronization Mode
} gtccr_t;

typedef struct
{
	// TIMSK0
	RW bool_t toie0   : 1; // Overflow Interrupt Enable
	RW bool_t ociea0  : 1; // Output Compare A Match Interrupt Enable
	RW bool_t ocieb0  : 1; // Output Compare B Match Interrupt Enable
	RO unused_t       : 5; // Unimplemented: Read as '0'
	// TIMSK1
	RW bool_t toie1   : 1; // Overflow Interrupt Enable
	RW bool_t ociea1  : 1; // Output Compare A Match Interrupt Enable
	RW bool_t ocieb1  : 1; // Output Compare B Match Interrupt Enable
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW bool_t icie1   : 1; // Input Capture Interrupt Enable
	RO unused_t       : 2; // Unimplemented: Read as '0'
	// TIMSK2
	RW bool_t toie2   : 1; // Overflow Interrupt Enable
	RW bool_t ociea2  : 1; // Output Compare A Match Interrupt Enable
	RW bool_t ocieb2  : 1; // Output Compare B Match Interrupt Enable
	RO unused_t       : 5; // Unimplemented: Read as '0'
} timsk_t;

typedef struct
{
	// TIFR0
	RW bool_t tov0    : 1; // Timer/Counter0, Overflow Flag
	RW bool_t ocfa0   : 1; // Timer/Counter0, Output Compare A Match Flag
	RW bool_t ocfb0   : 1; // Timer/Counter0, Output Compare B Match Flag
	RO unused_t       : 5; // Unimplemented: Read as '0'
	// TIFR1
	RW bool_t tov1    : 1; // Timer/Counter1, Overflow Flag
	RW bool_t ocfa1   : 1; // Timer/Counter1, Output Compare A Match Flag
	RW bool_t ocfb1   : 1; // Timer/Counter1, Output Compare B Match Flag
	RO unused_t       : 2; // Unimplemented: Read as '0'
	RW bool_t icf1    : 1; // Timer/Counter1, Input Capture Flag
	RO unused_t       : 2; // Unimplemented: Read as '0'
	// TIFR2
	RW bool_t tov2    : 1; // Timer/Counter2, Overflow Flag
	RW bool_t ocfa2   : 1; // Timer/Counter2, Output Compare A Match Flag
	RW bool_t ocfb2   : 1; // Timer/Counter2, Output Compare B Match Flag
	RO unused_t       : 5; // Unimplemented: Read as '0'
} tifr_t;


SFR_REG(0x24) timer0_t timer0;
SFR_REG(0x90) timer2_t timer2;
SFR_REG(0x60) timer1_t timer1;
SFR_REG(0x23) gtccr_t  gtccr;
SFR_REG(0x4E) timsk_t  timsk;
SFR_REG(0x15) tifr_t   tifr;


#endif