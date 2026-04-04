#include "common.h"
#include "delays.h"
#include "interrupts.h"
#include "hal_port.h"
#include "uart.h"
#include "rfid.h"


#define BUILTIN_LED				ports.b.o5

#define LED_ON()				BUILTIN_LED = HIGH
#define LED_OFF()				BUILTIN_LED = LOW

#define CMD_GET_COUNT_KEY		0x04
#define CMD_GET_NEXT_KEY		0x08
#define CMD_RESET_QUEUE			0x10
#define CMD_WRITE_T55X7			0x20
#define CMD_WRITE_EM4X05		0x40
#define CMD_WRITE_KEY_MASK		0x03

#define RESP_SUCCESS			0x00
#define RESP_INVALID_COMMAND	0x81
#define RESP_KEY_QUEUE_EMPTY	0x82
#define RESP_INVALID_LENGTH		0x83
#define RESP_INVALID_VALUE		0x84

#define ENCODE_OFFSET			0x47
#define UART_READ_TIMEOUT_MS	200
#define KEYS_BUFFER_SIZE		10


MAIN()
{
	INIT()
	{
		/*
		 * PB7 = I - XTAL2
		 * PB6 = I - XTAL1
		 * PB5 = O - Builtin LED
		 * PB4 = I - Unused
		 * PB3 = O - RFID out 125kHz
		 * PB2 = I - Unused
		 * PB1 = I - Unused
		 * PB0 = I - Unused
		 */
		ports.b.out = 0b00000000;
		ports.b.dir = 0b00101000;
		
		/*
		 * PC7 = X - Unimplemented
		 * PC6 = I - RESET
		 * PC5 = I - Unused
		 * PC4 = I - Unused
		 * PC3 = I - Unused
		 * PC2 = I - Unused
		 * PC1 = I - Unused
		 * PC0 = I - Unused
		 */
		ports.c.out = 0b00000000;
		ports.c.dir = 0b00000000;
		
		/*
		 * PD7 = I - Unused
		 * PD6 = I - Unused
		 * PD5 = I - Unused
		 * PD4 = I - Unused
		 * PD3 = I - Unused
		 * PD2 = I - RFID in
		 * PD1 = I - Tx Uart
		 * PD0 = I - Rx Uart
		 */
		ports.d.out = 0b00000000;
		ports.d.dir = 0b00000000;
		
		uart_init(9600UL, UART_8N1);
		rfid_init();
	}
	
	uint8_t keys_head = 0;
	uint8_t keys_tail = 0;
	uint8_t keys[KEYS_BUFFER_SIZE*RFID_KEY_LENGTH];
	
	rfid_read_start();
	
	MAIN_LOOP()
	{
		if (keys_tail < KEYS_BUFFER_SIZE)
		{
			uint8_t *key = keys + keys_tail * RFID_KEY_LENGTH;
			uint8_t status = rfid_read_key(key);
			
			if (status != KEY_STATUS_WAIT)
			{
				if (status == KEY_STATUS_VALID)
				{
					keys_tail++;
				}
				
				rfid_read_start();
			}
		}
		
		if (uart_available())
		{
			uint8_t command = uart_read_byte();
			
			if (command == CMD_GET_COUNT_KEY)
			{
				uint8_t count = keys_tail - keys_head;
				uart_write_byte(count);
				uart_write_byte(~count);
			}
			else if (command == CMD_GET_NEXT_KEY)
			{
				if (keys_tail != keys_head && keys_head < KEYS_BUFFER_SIZE)
				{
					uint8_t *key = keys + keys_head * RFID_KEY_LENGTH;
					uint8_t crc  = 0;
					for (uint8_t i = 0; i < RFID_KEY_LENGTH; i++)
					{
						uint8_t byte = *key++;
						uart_write_byte(ENCODE_OFFSET + (byte >> NIBBLE_BITS));
						uart_write_byte(ENCODE_OFFSET + (byte & NIBBLE_LOW));
						crc += byte;
					}
					crc = (~crc) + 1;
					uart_write_byte(ENCODE_OFFSET + (crc >> NIBBLE_BITS));
					uart_write_byte(ENCODE_OFFSET + (crc & NIBBLE_LOW));
					
					uint8_t ack = 0;
					if (uart_read(UART_READ_TIMEOUT_MS, ack) && ack == crc)
					{
						if (++keys_head == keys_tail)
						{
							keys_head = 0;
							keys_tail = 0;
							
							rfid_read_start();
						}
						
						uart_write_byte(RESP_SUCCESS);
					}
				}
				else
				{
					uart_write_byte(RESP_KEY_QUEUE_EMPTY);
				}
			}
			else if (command == CMD_RESET_QUEUE)
			{
				keys_head = 0;
				keys_tail = 0;
				
				rfid_read_start();
				
				uart_write_byte(RESP_SUCCESS);
			}
			else if (command & (CMD_WRITE_T55X7 | CMD_WRITE_EM4X05))
			{
				uint8_t buffer[2*WRITE_KEYS_MAX*RFID_KEY_LENGTH];
				uint8_t count = command & CMD_WRITE_KEY_MASK;
				
				if (uart_read_bytes(UART_READ_TIMEOUT_MS, buffer, 2*(count*RFID_KEY_LENGTH+1)))
				{
					uint8_t *pbuffer = &buffer[0];
					uint8_t crc = 0;
					
					for (uint8_t i = 0; i <= count*RFID_KEY_LENGTH; i++)
					{
						uint8_t h = *pbuffer++ - ENCODE_OFFSET;
						uint8_t l = *pbuffer++ - ENCODE_OFFSET;
						uint8_t b = (h << NIBBLE_BITS) | l;
						buffer[i] = b;
						crc += b;
					}
					
					if (crc == 0)
					{
						LED_ON();
						INT_DISABLE();
						
						if (command & CMD_WRITE_T55X7)
						{
							rfid_t55x7_write_keys(buffer, count);
						}
						else if (command & CMD_WRITE_EM4X05)
						{
							rfid_em4x05_write_key(buffer);
						}
						uart_write_byte(RESP_SUCCESS);
						
						keys_head = 0;
						keys_tail = 0;
						rfid_read_start();
						
						LED_OFF();
						INT_ENABLE();
					}
					else
					{
						uart_write_byte(RESP_INVALID_VALUE);
					}
				}
				else
				{
					uart_write_byte(RESP_INVALID_LENGTH);
				}
			}
			else
			{
				uart_write_byte(RESP_INVALID_COMMAND);
			}
		}
	}
}