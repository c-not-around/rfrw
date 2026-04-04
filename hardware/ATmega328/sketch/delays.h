#ifndef _DELAYS_H
#define _DELAYS_H


#include <util/delay.h>
#include "types.h"


#define delay_us(t)	_delay_us(t)
#define delay_ms(t)	_delay_ms(t)


void rt_delay_us(uint16_t t);


#endif