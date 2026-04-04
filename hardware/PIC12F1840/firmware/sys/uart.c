#include "common.h"
#include "delays.h"
#include "hal_int.h"
#include "hal_uart.h"
#include "uart.h"


#define BAUD_RATE			9600UL

#define TIMEOUT_STEP_US		4
#define TIMEOUT_CYCLES		(1000/TIMEOUT_STEP_US)


void uart_init()
{
	uart.txen = TRUE;
	uart.cren = TRUE;
	uart.spen = TRUE;
	
	/*
	baud	spbrg
	1200	0x1A09
	2400	0x0D04
	4800	0x0681
	9600	0x0340
	19200	0x019F
	38400	0x00CF
	57600	0x0089
	115200	0x0044
	*/
#define SPBRG_0		(_XTAL_FREQ/(64UL*BAUD_RATE)-1UL)
#define SPBRG_12	(_XTAL_FREQ/(16UL*BAUD_RATE)-1UL)
#define SPBRG_3		(_XTAL_FREQ/(4UL*BAUD_RATE)-1UL)

#define BAUD_0		(_XTAL_FREQ/(64UL*(SPBRG_0+1UL)))
#define BAUD_12		(_XTAL_FREQ/(16UL*(SPBRG_12+1UL)))
#define BAUD_3		(_XTAL_FREQ/(4UL*(SPBRG_3+1UL)))

#define ERR_0		(10000UL*(BAUD_0-BAUD_RATE)/BAUD_RATE)
#define ERR_12		(10000UL*(BAUD_12-BAUD_RATE)/BAUD_RATE)
#define ERR_3		(10000UL*(BAUD_3-BAUD_RATE)/BAUD_RATE)

#if ERR_0 < ERR_12 && ERR_0 < ERR_3
	uart.brgh  = FALSE;
	uart.brg16 = FALSE;
	uart.spbrg = SPBRG_0;				
#elif ERR_3 < ERR_12 && ERR_3 < ERR_0
	uart.brgh  = TRUE;
	uart.brg16 = TRUE;
	uart.spbrg = SPBRG_3;		
#else
	uart.brgh  = FALSE;
	uart.brg16 = TRUE;
	uart.spbrg = SPBRG_12;				
#endif
	
	intpie.txie = FALSE;
	intpie.rxie = FALSE;
}


bool_t uart_available()
{
	return intpir.rxif ? TRUE : FALSE;
}

uint8_t uart_read_byte()
{
	while (!intpir.rxif);
	return uart.rxbuf;
}

bool_t uart_read_bytes(uint16_t timeout, uint8_t *data, uint8_t size)
{
	uint8_t t = TIMEOUT_CYCLES;
	
	while (size && timeout)
	{
		if (intpir.rxif)
		{
			*data++ = uart.rxbuf;
			size--;
		}
		else
		{
			delay_us(TIMEOUT_STEP_US);
		}
		
		if (--t == 0)
		{
			t = TIMEOUT_CYCLES;
			timeout--;
		}
	}
	
	return size ? FALSE : TRUE;
}


void uart_write_byte(const uint8_t data)
{
	while (!intpir.txif);
	uart.txbuf = data;
}

void uart_write_bytes(const uint8_t *data, uint8_t size)
{
	while (size--)
	{
		while (!intpir.txif);
		uart.txbuf = *data++;
	}
}

void uart_write_byte_hex(const uint8_t data)
{
	uart_write_byte(HEX(data >> NIBBLE_BITS));
	uart_write_byte(HEX(data & NIBBLE_LOW));
}

void uart_write_bytes_hex(const uint8_t *data, uint8_t size)
{
	data += size;
	while (size--)
	{
		uart_write_byte_hex(*(--data));
	}
}

void uart_write_string(const char *text)
{
	while (*text)
	{
		uart_write_char(*text++);
	}
}