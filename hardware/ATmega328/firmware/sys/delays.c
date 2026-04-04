#include "common.h"
#include "delays.h"


/**
for 4MHz:
	t <= 4: 8+4=12 => 3us
	t >= 5: 8+5+4(t-4)+3=4t-16+16=4t => (t)us
for 8MHz:
	t <= 2: 8+4=12 => 1.5us
	t >= 3: 8+7+4(2t-4)+3=8t-16+18=8t+2 => (t+0.25)us
for 12MHz:
	t <= 1: 8+4=12 => 1us
	t >= 2: 8+10+4(3t-5)+3=12t-20+21=12t+1 => (t+0.083...)us
for 16MHz:
	t <= 1: 8+4=12 => 0.75us
	t >= 2: 8+9+4(4t-5)+3=16t-20+20=16t => (t)us
for 20MHz:
	t <= 1: 8+9=17 => 0.85us
	t >= 2: 8+17+4(5t-7)+3=20t-28+28=20t => (t)us
for 24MHz:
	t == 0: 8+4=13 => 0.5416...us
	t >= 1: 8+13+4(6t-6)+3=24t-24+24=24t => (t)us
**/
void rt_delay_us(uint16_t t)
{
	// LD                // 2 cycles
	// ST                // 2 cycles
	// CALL              // 4 cycles
#if F_CPU == (24*MHZ)
	asm volatile
	(
		"nop"
	);                   // 1 cycle
	if (!us) return;     // 3 cycles (4 when true)
	t *= 6;              // 7 cycles
	t -= 6;              // 2 cycles
#elif F_CPU == (20*MHZ)
	asm volatile
	(
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop"
	);                   // 5 cycles
	if (t <= 1) return;  // 3 cycles (4 when true)
	t = (t << 2) + t;    // 7 cycles
	t -= 7;              // 2 cycles
#elif F_CPU == (16*MHZ)
	if (t <= 1) return;  // 3 cycles (4 when true)
	t <<= 2;             // 4 cycles
	t -= 5;              // 2 cycles
#elif F_CPU == (12*MHZ)
	if (t <= 1) return;  // 3 cycles (4 when true)
	t = (t << 1) + t;    // 5 cycles
	t -= 5;              // 2 cycles
#elif F_CPU == (8*MHZ)
	if (t <= 2) return;  // 3 cycles (4 when true)
	t <<= 1;             // 2 cycles
	t -= 4;              // 2 cycles
#elif F_CPU == (4*MHZ)
	if (t <= 4) return;  // 3 cycles (4 when true)
	t -= 4;              // 2 cycles
#endif
	asm volatile         // 4 cycles (3 when reached zero)
	(
		"1: sbiw %0,1" "\n\t"
		"brne 1b" : "=w" (t) : "0" (t)
	);
	// RET               // 4 cycles
}