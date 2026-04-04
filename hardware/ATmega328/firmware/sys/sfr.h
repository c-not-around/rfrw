#ifndef _SFR_H
#define _SFR_H


#define SFR_OFFSET		0x20
#define SFR_REG(a)		volatile __attribute__((address(SFR_OFFSET+a)))


#endif