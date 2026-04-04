#include "common.h"
#include "delays.h"


void rt_delay_us(volatile uint8_t t)
{
	//									; t>2			t=1		t=2
	//	MOVLW	<constant>				; 1				1		1
	//	MOVF	<valiable>, W
	//	FCALL	_rt_delay_us			; 3				3		3
	#asm
	WREG	EQU	009h
		DECFSZ	WREG, F					; 1				2		1
		GOTO	delay_us_sub2			; 2						2
		RETURN							; 				2
	delay_us_sub2:
		DECFSZ	WREG, F					; 1						2
		GOTO	delay_us_cycle_init		; 2
		GOTO	$+1						; 						2
		NOP								; 						1
		GOTO	$+1						; 						2
		RETURN							;						2
	delay_us_cycle_init:
		GOTO	$+1						; 2
		NOP								; 1
	delay_us_cycle:
		GOTO	$+1						; 2*t
		NOP								; 1*t
		GOTO	$+1						; 2*t
		DECFSZ	WREG, F					; 1*t+1
		GOTO	delay_us_cycle			; 2*t
	#endasm
	//	RETURN							; 2
	//									; 16+8(t-2)		8		16
}

void rt_delay_ms(uint8_t t)
{
	//	BANKSEL							; 1*t
	//	MOVWF	rt_delay_ms@t			; 1*t
	
	rt_delay_us(250); //				; 250*8*t
	rt_delay_us(250); //				; 250*8*t
	rt_delay_us(250); //				; 250*8*t
	rt_delay_us(249); //				; 249*8*t
	
	t--;
	
	if (t)
	{
	#asm
		GOTO	_rt_delay_ms
	#endasm
	}
	
	//	DECF	rt_delay_ms@t,f			; 1*t
	//	MOVF	rt_delay_ms@t,w			; 1*t
	//	BTFSC	3,2						; 2*t
	//	RETURN							; 2
	//	GOTO	rt_delay_ms_cycle		; 2*t
	//									; 999*8*t + 8*t + 2 = 8000*t + 2
}