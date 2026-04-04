#ifndef _CONFIG_H
#define _CONFIG_H


#include <pic.h>


/*
	WARNING!
	The header file (supplied with the HI-TECH PICC 9.71a compiler) for the PIC12F1840 contains an mistake. 
	To correct the definition of LVP_OFF, you need to make the following change to this file:
	C:\Program Files (x86)\HI-TECH Software\PICC\9.71a\include\pic12f1840.h
	line 107
--	#define PLLEN_OFF            0xFDFF
++	#define PLLEN_OFF            0xFEFF
	line 112
--	#define STVREN_OFF           0xFBFF
++	#define STVREN_OFF           0xFDFF
	line 117
--	#define BORV_25              0xF7FF
++	#define BORV_25              0xFBFF
	line 122
--	#define LVP_OFF              0xBFFF
++	#define LVP_OFF              0xDFFF
*/
#if PLLEN_OFF == 0xFDFF
#error incorrect define for PLLEN_OFF.
#endif
#if STVREN_OFF == 0xFBFF
#error incorrect define for STVREN_OFF.
#endif
#if BORV_25 == 0xF7FF
#error incorrect define for BORV_25.
#endif
#if LVP_OFF == 0xBFFF
#error incorrect define for LVP_OFF.
#endif


/* CONFIG WORD1:
 * bit13: FCMEN
 * bit12: IESO
 * bit11: #CLKOUTEN
 * bit10: BOREN1
 * bit9 : BOREN0
 * bit8 : #CPD
 * bit7 : #CP
 * bit6 : MCLRE
 * bit5 : #PWRTE
 * bit4 : WDTE1
 * bit3 : WDTE0
 * bit2 : FOSC2	LP/XT/HS/EXTRC/INTOSC/ECL/ECM/ECH
 * bit1 : FOSC1
 * bit0 : FOSC0
 */
__CONFIG(FCMEN_OFF & IESO_OFF & CLKOUTEN_OFF & BOREN_OFF & CPD_OFF & CP_OFF & MCLRE_OFF & PWRTE_ON & WDTE_OFF & FOSC_INTOSC);

/* CONFIG WORD2:
 * bit13: LVP
 * bit12: #DEBUG
 * bit11: unimplemented - read as 0
 * bit10: BORV
 * bit9 : STVREN
 * bit8 : PLLEN
 * bit7 : unimplemented - read as 0
 * bit6 : unimplemented - read as 0
 * bit5 : unimplemented - read as 0
 * bit4 : reserved - read as 1
 * bit3 : unimplemented - read as 0
 * bit2 : unimplemented - read as 0
 * bit1 : WRT1
 * bit0 : WRT0
 */
__CONFIG(LVP_OFF & BORV_25 & STVREN_ON & PLLEN_ON & WRT_OFF);


#endif