#ifndef _HAL_EEP_H
#define _HAL_EEP_H


//eep.co1 - all settings
#define EECON1_WREN_DISABLE		(0b0 << 2)
#define EECON1_WREN_ENABLE		(0b1 << 2)
#define EECON1_FREE_DISABLE		(0b0 << 4)
#define EECON1_FREE_ENABLE		(0b1 << 4)
#define EECON1_LWLO_LOAD_WRITE	(0b0 << 5)
#define EECON1_LWLO_LOAD_ONLY	(0b1 << 5)
#define EECON1_CFGS_FLASH		(0b0 << 6)
#define EECON1_CFGS_CONFIG		(0b1 << 6)
#define EECON1_EEPGD_EEPROM		(0b0 << 7)
#define EECON1_EEPGD_FLASH		(0b1 << 7)
//eep.co1 - all settings
#define EECON2_WHITE_MAGIC		0x55
#define EECON2_BLACK_MAGIC		0xAA


typedef struct
{
	// EEADRL
	union
	{
		struct
		{
			RW uint8_t adrl;
			RW uint8_t adrh;
		};
		RW uint16_t adr;
	};
	// EEDATL
	union
	{
		struct
		{
			RW uint8_t datl;
			RW uint8_t dath;
		};
		RW uint16_t dat;
	};
	// EECON1
	union
	{
		struct
		{
			RW bool_t rd    : 1; // Read Control bit
			RW bool_t wr    : 1; // Write Control bit
			RW bool_t wren  : 1; // Program/Erase Enable bit
			RW bool_t wrerr : 1; // EEPROM Error Flag bit
			RW bool_t free  : 1; // Program Flash Erase Enable bit
			RW bool_t lwlo  : 1; // Load Write Latches Only bit
			RW bool_t cfgs  : 1; // Flash Program/Data EEPROM or Configuration Select bit
			RW bool_t eepgd : 1; // Flash Program/Data EEPROM Memory Select bit
		};
		RW uint8_t con1;
	};
	// EECON2
	RW uint8_t con2;
} eep_t;


SFR eep_t eep @ 0x191;


#endif