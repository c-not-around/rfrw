#ifndef _HAL_INT_H
#define _HAL_INT_H


#include "types.h"


// intcon.con - all settings
#define INTCON_IOCIF_CLEAR		(0b0 << 0)
#define INTCON_IOCIF_SET		(0b1 << 0)
#define INTCON_INTF_CLEAR		(0b0 << 1)
#define INTCON_INTF_SET			(0b1 << 1)
#define INTCON_TMR0IF_CLEAR		(0b0 << 2)
#define INTCON_TMR0IF_SET		(0b1 << 2)
#define INTCON_IOCIE_DISABLE	(0b0 << 3)
#define INTCON_IOCIE_ENABLE		(0b1 << 3)
#define INTCON_INTE_DISABLE		(0b0 << 4)
#define INTCON_INTE_ENABLE		(0b1 << 4)
#define INTCON_TMR0IE_DISABLE	(0b0 << 5)
#define INTCON_TMR0IE_ENABLE	(0b1 << 5)
#define INTCON_PEIE_DISABLE		(0b0 << 6)
#define INTCON_PEIE_ENABLE		(0b1 << 6)
#define INTCON_GIE_DISABLE		(0b0 << 7)
#define INTCON_GIE_ENABLE		(0b1 << 7)

#define INT_DISABLE()			intcon.gie = FALSE
#define INT_ENABLE()			intcon.gie = TRUE

#define INT_HANDLER()			void interrupt isr()


typedef union
{
	// INTCON
	struct
	{
		RW bool_t iocif  : 1; // Interrupt-on-Change Interrupt Flag bit
		RW bool_t intf   : 1; // INT External Interrupt Flag bit
		RW bool_t tmr0if : 1; // Timer0 Overflow Interrupt Flag bit
		RW bool_t iocie  : 1; // Interrupt-on-Change Enable bit
		RW bool_t inte   : 1; // INT External Interrupt Enable bit
		RW bool_t tmr0ie : 1; // Timer0 Overflow Interrupt Enable bit
		RW bool_t peie   : 1; // Peripheral Interrupt Enable bit
		RW bool_t gie    : 1; // Global Interrupt Enable bit
	};
	RW uint8_t con;
} intcon_t;

typedef union
{
	struct
	{
		// PIR1
		RW bool_t tmr1if  : 1; // Timer1 Overflow Interrupt Flag bit
		RW bool_t tmr2if  : 1; // Timer2 to PR2 Interrupt Flag bit
		RW bool_t ccp1if  : 1; // CCP1 Interrupt Flag bit
		RW bool_t spp1if  : 1; // Synchronous Serial Port (MSSP) Interrupt Flag bit
		RO bool_t txif    : 1; // USART Transmit Interrupt Flag bit
		RO bool_t rxif    : 1; // USART Receive Interrupt Flag bit
		RW bool_t adif    : 1; // A/D Converter Interrupt Flag bit
		RW bool_t tmr1gif : 1; // Timer1 Gate Interrupt Flag bit
		// PIR2
		RO unused_t       : 3; // Unimplemented: Read as '0'
		RW bool_t bcl1if  : 1; // MSSP Bus Collision Interrupt Flag bit
		RW bool_t eeif    : 1; // EEPROM Write Completion Interrupt Flag bit
		RW bool_t c1if    : 1; // Comparator C1 Interrupt Flag bit
		RO unused_t       : 1; // Unimplemented: Read as '0'
		RW bool_t osfif   : 1; // Oscillator Fail Interrupt Flag bit
	};
	struct
	{
		RW uint8_t con1;
		RW uint8_t con2;
	};
	RW uint16_t con;
} intpir_t;

typedef union
{
	struct
	{
		// PIE1
		RW bool_t tmr1ie  : 1; // Timer1 Overflow Interrupt Enable bit
		RW bool_t tmr2ie  : 1; // TMR2 to PR2 Match Interrupt Enable bit
		RW bool_t ccp1ie  : 1; // CCP1 Interrupt Enable bit
		RW bool_t spp1ie  : 1; // Synchronous Serial Port (MSSP) Interrupt Enable bit
		RW bool_t txie    : 1; // USART Transmit Interrupt Enable bit
		RW bool_t rxie    : 1; // USART Receive Interrupt Enable bit
		RW bool_t adie    : 1; // A/D Converter (ADC) Interrupt Enable bit
		RW bool_t tmr1gie : 1; // Timer1 Gate Interrupt Enable bit
		// PIE2
		RO unused_t       : 3; // Unimplemented: Read as '0'
		RW bool_t bcl1ie  : 1; // MSSP Bus Collision Interrupt Enable bit
		RW bool_t eeie    : 1; // EEPROM Write Completion Interrupt Enable bit
		RW bool_t c1ie    : 1; // Comparator C1 Interrupt Enable bit
		RO unused_t       : 1; // Unimplemented: Read as '0'
		RW bool_t osfie   : 1; // Oscillator Fail Interrupt Enable bit
	};
	struct
	{
		RW uint8_t con1;
		RW uint8_t con2;
	};
	RW uint16_t con;
} intpie_t;

typedef struct
{
	// IOCAP
	union
	{
		struct
		{
			RW bool_t iocap0 : 1; // Interrupt-on-Change PORTA Positive Edge Enable bits
			RW bool_t iocap1 : 1;
			RW bool_t iocap2 : 1;
			RW bool_t iocap3 : 1;
			RW bool_t iocap4 : 1;
			RW bool_t iocap5 : 1;
			RO unused_t      : 2;
		};
		RW uint8_t con;
	} iocap;
	// IOCAN
	union
	{
		struct
		{
			RW bool_t iocan0 : 1; // Interrupt-on-Change PORTA Negative Edge Enable bits
			RW bool_t iocan1 : 1;
			RW bool_t iocan2 : 1;
			RW bool_t iocan3 : 1;
			RW bool_t iocan4 : 1;
			RW bool_t iocan5 : 1;
			RO unused_t      : 2;
		};
		RW uint8_t con;
	} iocan;
	// IOCAF
	union
	{
		struct
		{
			RW bool_t iocaf0 : 1; // Interrupt-on-Change PORTA Flag bits
			RW bool_t iocaf1 : 1;
			RW bool_t iocaf2 : 1;
			RW bool_t iocaf3 : 1;
			RW bool_t iocaf4 : 1;
			RW bool_t iocaf5 : 1;
			RO unused_t      : 2;
		};
		RW uint8_t con;
	} iocaf;
} intext_t;


SFR intcon_t intcon @ 0x00B;
SFR intpir_t intpir @ 0x011;
SFR intpie_t intpie @ 0x091;
SFR intext_t intext @ 0x391;


#endif