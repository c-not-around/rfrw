#include "config.h"
#include "common.h"
#include "delays.h"
#include "hal_osc.h"
#include "hal_opt.h"
#include "hal_cmp.h"
#include "hal_cps.h"
#include "hal_fvr.h"
#include "hal_apf.h"
#include "hal_int.h"
#include "hal_port.h"
#include "uart.h"
#include "boot.h"
#include "rfid.h"


#define BUILTIN_LED				poa.o2

#define LED_ON()				BUILTIN_LED = HIGH
#define LED_OFF()				BUILTIN_LED = LOW

#define CMD_START_BOOTLOADER	0x02
#define CMD_GET_COUNT_KEY		0x04
#define CMD_GET_NEXT_KEY		0x08
#define CMD_RESET_QUEUE			0x10
#define CMD_WRITE_T55X7			0x20
#define CMD_WRITE_EM4X05		0x40

#define CMD_WRITE_KEY_MASK		0x03
#define CMD_BOOTLOADER_PASS		0x5AA5

#define RESP_SUCCESS			0x00
#define RESP_INVALID_COMMAND	0x81
#define RESP_KEY_QUEUE_EMPTY	0x82
#define RESP_INVALID_LENGTH		0x83
#define RESP_INVALID_VALUE		0x84
#define RESP_INVALID_PASS		0x85

#define ENCODE_OFFSET			0x47
#define UART_READ_TIMEOUT_MS	200
#define KEYS_BUFFER_SIZE		10


MAIN()
{
	INIT()
	{
		osc.con  = OSCCON_PLL_ENABLE | OSCCON_IRCF_8_OR_32MHZ_HF;
		opt.con  = OPTCON_WPU_ENABLE;
		
		cm1.con0 = CMPCON0_CMP_OFF; 
		cps.con0 = CPSCON0_CPS_OFF;
		fvr.con  = FVRCON_FVR_OFF;
		
		/*
		 * RA7 = X - not implemented
		 * RA6 = X - not implemented
		              Default | RDM6300  | ICSP
		 * RA5 = I - RFID_OUT | RX       |
		 * RA4 = I - RFID_IN  | TX       |
		 * RA3 = I - BUTTON   |          | VPP
		 * RA2 = O - LED      | RFID_OUT |
		 * RA1 = I - RX       | LED      | PGC
		 * RA0 = I - TX       | RFID_IN  | PGD
		 */
#ifdef PLATFORM_RDM6300
		apf.con    = APFCON_RXDT_RA5 | APFCON_TXCK_RA4;
		pda.dir    = 0b11111101;
#else
		apf.con    = APFCON_CCP1_RA5;
		pda.dir    = 0b11111011;
#endif
		poa.out    = 0b00000000;
		paa.analog = 0b00000000;
		ppa.pullup = 0b00001000;
		
		intcon.con = INTCON_GIE_DISABLE | INTCON_PEIE_DISABLE | INTCON_IOCIE_DISABLE;
		
#ifndef PLATFORM_RDM6300
		if (BOOT_JUMPER == LOW)
		{
			JUMP_TO_BOOT();
		}
#endif
		
		uart_init();
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
#ifndef PLATFORM_RDM6300
			else if (command == CMD_START_BOOTLOADER)
			{
				uart_write_byte(RESP_SUCCESS);
				
				uint16_t pass;
				if (uart_read(UART_READ_TIMEOUT_MS, pass) && pass == CMD_BOOTLOADER_PASS)
				{
					uart_write_byte(RESP_SUCCESS);
					
					INT_DISABLE();
					JUMP_TO_BOOT();
				}
				else
				{
					uart_write_byte(RESP_INVALID_PASS);
				}
			}
#endif
			else
			{
				uart_write_byte(RESP_INVALID_COMMAND);
			}
		}
	}
}