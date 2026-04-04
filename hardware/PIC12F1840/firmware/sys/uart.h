#ifndef _UART_H
#define _UART_H


#include "types.h"


#define uart_write_char(c)		uart_write_byte((uint8_t)(c))
#define uart_read(t,v)			uart_read_bytes(t,(uint8_t*)&(v),sizeof(v))
#define uart_write(v)			uart_write_bytes((uint8_t*)&(v),sizeof(v))
#define uart_write_hex(v)		uart_write_bytes_hex((uint8_t*)&(v),sizeof(v))


void uart_init();

bool_t uart_available();
uint8_t uart_read_byte();
bool_t uart_read_bytes(uint16_t timeout, uint8_t *data, uint8_t size);

void uart_write_byte(const uint8_t data);
void uart_write_bytes(const uint8_t *data, uint8_t size);
void uart_write_byte_hex(const uint8_t data);
void uart_write_bytes_hex(const uint8_t *data, uint8_t size);
void uart_write_string(const char *text);


#endif