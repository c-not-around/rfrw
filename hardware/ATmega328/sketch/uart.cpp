#include "common.h"
#include "delays.h"
#include "hal_uart.h"
#include "uart.h"


#define UART_SETTINGS_MASK	0b00111110

#define TIMEOUT_STEP_US		4
#define TIMEOUT_CYCLES		(1000/TIMEOUT_STEP_US)


void uart_init(const uint32_t baudrate, const uint8_t settings)
{
	ucsr.ubrr  = F_CPU / (16 * baudrate) - 1;
	ucsr.ucsrc = settings & UART_SETTINGS_MASK;
	ucsr.txen  = TRUE;
	ucsr.rxen  = TRUE;
}


bool_t uart_available()
{
	return ucsr.rxc ? TRUE : FALSE;
}

uint8_t uart_read_byte()
{
	while (!ucsr.rxc);
	return ucsr.udr;
}

bool_t uart_read_bytes(uint16_t timeout, uint8_t *data, uint8_t size)
{
	uint8_t t = TIMEOUT_CYCLES;
	
	while (size && timeout)
	{
		if (ucsr.rxc)
		{
			*data++ = ucsr.udr;
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
	while (!ucsr.udre);
	ucsr.udr = data;
}

void uart_write_bytes(const uint8_t *data, uint8_t size)
{
	while (size--)
	{
		while (!ucsr.udre);
		ucsr.udr = *data++;
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