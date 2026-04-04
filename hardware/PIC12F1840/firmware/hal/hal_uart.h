#ifndef _HAL_UART_H
#define _HAL_UART_H


#include "types.h"


// uart.rcsta - all settings
#define RCSTA_ADR_DETECT_OFF	(0b0 << 3)
#define RCSTA_ADR_DETECT_ON		(0b1 << 3)
#define RCSTA_RX_CONTINUOUS_OFF	(0b0 << 4)
#define RCSTA_RX_CONTINUOUS_ON	(0b1 << 4)
#define RCSTA_RX_SINGLE_OFF		(0b0 << 5)
#define RCSTA_RX_SINGLE_ON		(0b1 << 5)
#define RCSTA_RX_9BIT_OFF		(0b0 << 6)
#define RCSTA_RX_9BIT_ON		(0b1 << 6)
#define RCSTA_RX_OFF			(0b0 << 7)
#define RCSTA_RX_ON				(0b1 << 7)
// uart.txsta - all settings
#define TXSTA_BRGH_RATE_LOW		(0b0 << 2)
#define TXSTA_BRGH_RATE_HIGH	(0b1 << 2)
#define TXSTA_SENDB_OFF			(0b0 << 3)
#define TXSTA_SENDB_ON			(0b1 << 3)
#define TXSTA_SYNC_MODE_OFF		(0b0 << 4)
#define TXSTA_SYNC_MODE_ON		(0b1 << 4)
#define TXSTA_TX_DISABLE		(0b0 << 5)
#define TXSTA_TX_ENABLE			(0b1 << 5)
#define TXSTA_TX_9BIT_OFF		(0b0 << 6)
#define TXSTA_TX_9BIT_ON		(0b1 << 6)
#define TXSTA_CSRC_CLK_EXT		(0b0 << 7)
#define TXSTA_CSRC_CLK_INT		(0b1 << 7)
// uart.baudcon - all settings
#define BAUDCON_BAUD_DETECT_OFF	(0b0 << 0)
#define BAUDCON_BAUD_DETECT_ON	(0b1 << 0)
#define BAUDCON_WUE_WAKEUP_OFF	(0b0 << 1)
#define BAUDCON_WUE_WAKEUP_ON	(0b1 << 1)
#define BAUDCON_BRG16_OFF		(0b0 << 3)
#define BAUDCON_BRG16_ON		(0b1 << 3)
#define BAUDCON_SCKP_DIR_OFF	(0b0 << 4)
#define BAUDCON_SCKP_INV_ON		(0b1 << 4)


typedef struct
{
	// RCREG
	RW uint8_t rxbuf;  // EUSART Receive Data Register
	// TXREG
	RW uint8_t txbuf;  // USART Transmit Data Register
	// SPBRG
	RW uint16_t spbrg; // Baud Rate Generator Data Register
	// RCSTA
	union
	{
		struct
		{
			RO logic_t rx9d  : 1; // Ninth bit of Received Data
			RO bool_t  oerr  : 1; // Overrun Error bit
			RO bool_t  ferr  : 1; // Framing Error bit
			RW bool_t  adden : 1; // Address Detect Enable bit
			RW bool_t  cren  : 1; // Continuous Receive Enable bit
			RW bool_t  sren  : 1; // Single Receive Enable bit
			RW bool_t  rx9en : 1; // 9-bit Receive Enable bit
			RW bool_t  spen  : 1; // Serial Port Enable bit
		};
		RW uint8_t rcsta;
	};
	// TXSTA
	union
	{
		struct
		{
			RW logic_t tx9d  : 1; // Ninth bit of Transmit Data
			RO bool_t  trmt  : 1; // Transmit Shift Register Status bit
			RW bool_t  brgh  : 1; // High Baud Rate Select bit
			RW bool_t  sendb : 1; // Send Break Character bit
			RW bool_t  sync  : 1; // EUSART Mode Select bit
			RW bool_t  txen  : 1; // Transmit Enable bit
			RW bool_t  tx9en : 1; // 9-bit Transmit Enable bit
			RW bool_t  csrc  : 1; // Clock Source Select bit
		};
		RW uint8_t txsta;
	};
	// BAUDCON
	union
	{
		struct
		{
			RW bool_t abden  : 1; // Auto-Baud Detect Enable bit
			RW bool_t wue    : 1; // Wake-up Enable bit
			RO unused_t      : 1; // Unimplemented: Read as '0'
			RW bool_t brg16  : 1; // 16-bit Baud Rate Generator bit
			RW bool_t sckp   : 1; // Synchronous Clock Polarity Select bit
			RO unused_t      : 1; // Unimplemented: Read as '0'
			RO bool_t rcidl  : 1; // Receive Idle Flag bit
			RO bool_t abdovf : 1; // Auto-Baud Detect Overflow bit
		};
		RW uint8_t baudcon;
	};
} uart_t;


SFR uart_t uart @ 0x199;


#endif