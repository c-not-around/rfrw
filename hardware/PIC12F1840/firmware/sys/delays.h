#ifndef _DELAYS_H
#define _DELAYS_H


#include <htc.h>


#define delay_us(t)		__delay_us(t)
#define delay_ms(t)		__delay_ms(t)


void rt_delay_us(uint8_t t);
void rt_delay_ms(uint8_t t);


#endif