#ifndef _BOOT_H
#define _BOOT_H


#include "hal_port.h"


#define BOOT_VECTOR				0xF00
#define BOOT_JUMPER				pia.i3

#define JUMP_TO_BOOT()			asm("\tMOVLP "___mkstr(BOOT_VECTOR>>8)); \
								asm("\tGOTO  "___mkstr(BOOT_VECTOR&0x7FF))


#endif