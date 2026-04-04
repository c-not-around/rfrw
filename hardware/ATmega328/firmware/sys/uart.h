#ifndef _UART_H
#define _UART_H


#include "types.h"


#define UART_DATA_BITS_5	(0b00 << 1)
#define UART_DATA_BITS_6	(0b01 << 1)
#define UART_DATA_BITS_7	(0b10 << 1)
#define UART_DATA_BITS_8	(0b11 << 1)
#define UART_STOP_BITS_1	(0b0  << 3)
#define UART_STOP_BITS_2	(0b1  << 3)
#define UART_PARITY_NONE	(0b00 << 4)
#define UART_PARITY_EVEN	(0b10 << 4)
#define UART_PARITY_ODD 	(0b11 << 4)

#define UART_5N1			(UART_DATA_BITS_5|UART_PARITY_NONE|UART_STOP_BITS_1)
#define UART_5N2			(UART_DATA_BITS_5|UART_PARITY_NONE|UART_STOP_BITS_2)
#define UART_5E1			(UART_DATA_BITS_5|UART_PARITY_EVEN|UART_STOP_BITS_1)
#define UART_5E2			(UART_DATA_BITS_5|UART_PARITY_EVEN|UART_STOP_BITS_2)
#define UART_5O1			(UART_DATA_BITS_5|UART_PARITY_ODD|UART_STOP_BITS_1)
#define UART_5O2			(UART_DATA_BITS_5|UART_PARITY_ODD|UART_STOP_BITS_2)
#define UART_6N1			(UART_DATA_BITS_6|UART_PARITY_NONE|UART_STOP_BITS_1)
#define UART_6N2			(UART_DATA_BITS_6|UART_PARITY_NONE|UART_STOP_BITS_2)
#define UART_6E1			(UART_DATA_BITS_6|UART_PARITY_EVEN|UART_STOP_BITS_1)
#define UART_6E2			(UART_DATA_BITS_6|UART_PARITY_EVEN|UART_STOP_BITS_2)
#define UART_6O1			(UART_DATA_BITS_6|UART_PARITY_ODD|UART_STOP_BITS_1)
#define UART_6O2			(UART_DATA_BITS_6|UART_PARITY_ODD|UART_STOP_BITS_2)
#define UART_7N1			(UART_DATA_BITS_7|UART_PARITY_NONE|UART_STOP_BITS_1)
#define UART_7N2			(UART_DATA_BITS_7|UART_PARITY_NONE|UART_STOP_BITS_2)
#define UART_7E1			(UART_DATA_BITS_7|UART_PARITY_EVEN|UART_STOP_BITS_1)
#define UART_7E2			(UART_DATA_BITS_7|UART_PARITY_EVEN|UART_STOP_BITS_2)
#define UART_7O1			(UART_DATA_BITS_7|UART_PARITY_ODD|UART_STOP_BITS_1)
#define UART_7O2			(UART_DATA_BITS_7|UART_PARITY_ODD|UART_STOP_BITS_2)
#define UART_8N1			(UART_DATA_BITS_8|UART_PARITY_NONE|UART_STOP_BITS_1)
#define UART_8N2			(UART_DATA_BITS_8|UART_PARITY_NONE|UART_STOP_BITS_2)
#define UART_8E1			(UART_DATA_BITS_8|UART_PARITY_EVEN|UART_STOP_BITS_1)
#define UART_8E2			(UART_DATA_BITS_8|UART_PARITY_EVEN|UART_STOP_BITS_2)
#define UART_8O1			(UART_DATA_BITS_8|UART_PARITY_ODD|UART_STOP_BITS_1)
#define UART_8O2			(UART_DATA_BITS_8|UART_PARITY_ODD|UART_STOP_BITS_2)

#define uart_write_char(c)	uart_write_byte((uint8_t)(c))
#define uart_read(t,v)		uart_read_bytes(t,(uint8_t*)&(v),sizeof(v))
#define uart_write(v)		uart_write_bytes((uint8_t*)&(v),sizeof(v))
#define uart_write_hex(v)	uart_write_bytes_hex((uint8_t*)&(v),sizeof(v))


void uart_init(const uint32_t baudrate, const uint8_t settings);

bool_t uart_available();
uint8_t uart_read_byte();
bool_t uart_read_bytes(uint16_t timeout, uint8_t *data, uint8_t size);

void uart_write_byte(const uint8_t data);
void uart_write_bytes(const uint8_t *data, uint8_t size);
void uart_write_byte_hex(const uint8_t data);
void uart_write_bytes_hex(const uint8_t *data, uint8_t size);
void uart_write_string(const char *text);


#endif