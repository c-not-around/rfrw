#ifndef _HAL_UART_H
#define _HAL_UART_H


#include "types.h"
#include "sfr.h"


typedef struct
{
	// UCSR0A
	union
	{
		struct
		{
			RW bool_t mpcm  : 1; // Multi-processor Communication Mode
			RW bool_t u2x   : 1; // Double the USART Transmission Speed
			RO bool_t upe   : 1; // USART Parity Error
			RO bool_t dor   : 1; // Data OverRun
			RO bool_t fe    : 1; // Frame Error
			RO bool_t udre  : 1; // USART Data Register Empty
			RW bool_t txc   : 1; // USART Transmit Complete
			RO bool_t rxc   : 1; // USART Receive Complete
		};
		RW uint8_t ucsra;
	};
	// UCSR0B
	union
	{
		struct
		{
			RW bool_t txb8  : 1; // Transmit Data Bit 8
			RO bool_t rxb8  : 1; // Receive Data Bit 8
			RW bool_t ucsz2 : 1; // Character Size
			RW bool_t txen  : 1; // Transmitter Enable
			RW bool_t rxen  : 1; // Receiver Enable
			RW bool_t udrie : 1; // USART Data Register Empty Interrupt Enable
			RW bool_t txcie : 1; // TX Complete Interrupt Enable
			RW bool_t rxcie : 1; // RX Complete Interrupt Enable
		};
		RW uint8_t ucsrb;
	};
	// UCSR0C
	union
	{
		struct
		{
			RW enum
			{
				UCPOL_DIRECT  = 0b0,
				UCPOL_INVERSE = 0b1
			} ucpol : 1; // Clock Polarity
			RW enum
			{
				UCSZ_FIVE  = 0b00,
				UCSZ_SIX   = 0b01,
				UCSZ_SEVEN = 0b10,
				UCSZ_EIGHT = 0b11
			} ucsz  : 2; // USART Character Size / Data Order
			RW enum
			{
				USBS_ONE_BIT  = 0b0,
				USBS_TWO_BITS = 0b1
			} usbs  : 1; // USART Stop Bit Select
			RW enum
			{
				UPM_NONE = 0b00,
				UPM_EVEN = 0b10,
				UPM_ODD  = 0b11
			} upm   : 2; // USART Parity Mode
			RW enum
			{
				UMSEL_ASYNC = 0b00,
				UMSEL_SYNC  = 0b01,
				UMSEL_MSPI  = 0b11
			} umsel : 2; // USART Mode Select
		} _ucsrc;
		RW uint8_t ucsrc;
	};
	RW unused_t : 8;
	// UBRR0L
	RW uint16_t ubrr; // USART Baud Rate Register
	// UDR0
	RW uint8_t  udr;  // USART Data Register
} ucsr_t;


SFR_REG(0xA0) ucsr_t ucsr;


#endif