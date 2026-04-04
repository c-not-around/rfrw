#include "common.h"
#include "delays.h"
#include "interrupts.h"
#include "hal_int.h"
#include "hal_port.h"
#include "hal_timer.h"
#include "rfid_defs.h"
#include "rfid_t55x7.h"
#include "rfid_em4x05.h"
#include "rfid.h"


#define RFID_OUT_D					ports.b.d3
#define RFID_INPUT					ports.d.i2

#define TMR2_DIV					1
#define TMR2_OCR					(F_CPU/(2*TMR2_DIV*RFID_FREQ)-1)

#define CAPT_TMR_DIV				8
#define CAPT_TMR_LSB_US				(F_CPU/(CAPT_TMR_DIV*US_PER_SECOND))

#define RFID_FULL_BIT_US			(500*CAPT_TMR_LSB_US)
#define RFID_HALF_BIT_US			(RFID_FULL_BIT_US/2)
#define RFID_BIT_DEVIATION			0.25
#define RFID_FBIT_MIN_US			((uint16_t)(RFID_FULL_BIT_US*(1.0-RFID_BIT_DEVIATION)))
#define RFID_FBIT_MAX_US			((uint16_t)(RFID_FULL_BIT_US*(1.0+RFID_BIT_DEVIATION)))
#define RFID_HBIT_MIN_US			((uint16_t)(RFID_HALF_BIT_US*(1.0-RFID_BIT_DEVIATION)))
#define RFID_HBIT_MAX_US			((uint16_t)(RFID_HALF_BIT_US*(1.0+RFID_BIT_DEVIATION)))

#define BIT_ZERO					0
#define BIT_ONE						1
#define BIT_UNDEF					2
#define BIT_WAIT					3

#define RECEIVE_MODE_KEY			0
#define RECEIVE_MODE_BLOCK			1

#define PACKET_BITS					64
#define START_BITS					9
#define ROW_BITS					4
#define ROW_PARITY_BITS				1
#define COL_PARITY_ROWS				1
#define RFID_ROWS					((PACKET_BITS-START_BITS)/(ROW_BITS+ROW_PARITY_BITS))
#define ROW_PARITY_BIT				(ROW_BITS+ROW_PARITY_BITS)
#define STOP_BIT					(PACKET_BITS-1)
#define COL_PARITY_ROW				(RFID_ROWS-COL_PARITY_ROWS)
#define KEY_DATA_ROWS				(RFID_ROWS-COL_PARITY_ROWS)

#define PACKET_BITS_RESTART			(uint8_t)(-1)
#define CAPTURE_RESET_MASK			0x8300

#define PWM_INIT()					{RFID_OUT_D  = OUTPUT;           \
									 timer2.wgml = T_PWM_FAST_OCR_L; \
									 timer2.wgmh = T_PWM_FAST_OCR_H; \
									 timer2.coma = T_TOGGLE;         \
									 timer2.ocra = TMR2_OCR;}
#define CAPT_INIT()					{eicra.isc0  = ISC_CHANGE; \
									 eifr.int0   = FALSE;      \
									 INT_ENABLE();}

#define FIELD_ON()					{timer2.cs   = T2_DIV_1;   \
									 RFID_OUT_D  = OUTPUT;}
#define FIELD_OFF()					{timer2.cs   = T2_DISABLE; \
									 RFID_OUT_D  = INPUT;}

#define CAPTURE_DISABLE()			eimsk.int0   = FALSE
#define CAPTURE_ENABLE()			eimsk.int0   = TRUE
#define CAPTURE_CLEAR()				eifr.int0    = FALSE

#define CAPTURE_STOP()				{timer1.cs   = T_DISABLE;   \
									 timer1.tcnt = 0;           \
									 timsk.toie1 = FALSE;       \
									 tifr.tov1   = FALSE; }
#define CAPTURE_RESTART(len)		{timer1.cs   = T_DISABLE;   \
									 len         = timer1.tcnt; \
									 timer1.tcnt = 0;           \
									 timer1.cs   = T_DIV_8;     \
									 timsk.toie1 = TRUE;}
#define CAPTURE_RESET()				{rfid.control           &= CAPTURE_RESET_MASK; \
									 rfid.packet_bit_counter = 0;                  \
									 rfid.receive_row        = 0x00;}


typedef struct
{
	union
	{
		struct
		{
			uint8_t row_bit_counter  : 4;
			uint8_t row_counter      : 4;
			uint8_t half_bit_counter : 2;
			logic_t parity_bit       : 1;
			logic_t half_bit         : 1;
			uint8_t current_bit      : 2;
			bool_t  received_flag    : 1;
			uint8_t receive_mode     : 1;
		};
		uint16_t control;
	};
	uint8_t packet_bit_counter;
	uint8_t receive_row;
	uint8_t receive_rows[RFID_ROWS];
} rfid_t;


volatile rfid_t rfid;


void rfid_init()
{
	PWM_INIT();
	CAPT_INIT();
}


void rfid_read_start()
{
	rfid.control            = 0x0000;
	rfid.packet_bit_counter = 0;
	rfid.receive_row        = 0x00;
	
	FIELD_ON();
	
	CAPTURE_CLEAR();
	CAPTURE_ENABLE();
}

uint8_t rfid_read_key(uint8_t *key)
{
	if (rfid.received_flag)
	{
		rfid.received_flag = FALSE;
		
		for (uint8_t i = 0; i < KEY_DATA_ROWS; i++)
		{
			rfid.receive_rows[COL_PARITY_ROW] ^= rfid.receive_rows[i];
		}
		
		for (uint8_t i = 0, j = 0; i < RFID_KEY_LENGTH; i++)
		{
			key[i]  = rfid.receive_rows[j++] << ROW_BITS;
			key[i] |= rfid.receive_rows[j++];
		}
		
		if (rfid.receive_rows[COL_PARITY_ROW])
		{
			return KEY_STATUS_BAD_CRC;
		}
		
		return KEY_STATUS_VALID;
	}
	
	return KEY_STATUS_WAIT;
}


void rfid_t55x7_gap()
{
	FIELD_OFF();
	delay_us(T55X7_START_GAP_US);
	FIELD_ON();
}

void rfid_t55x7_write_bits(uint8_t data, uint8_t bits)
{
	uint8_t mask = 1 << (bits - 1);
	
	while (bits--)
	{
		delay_us(T55X7_BIT_ZERO_US);
		if (data & mask)
		{
			delay_us(T55X7_BIT_ONE_US-T55X7_BIT_ZERO_US);
		}
		FIELD_OFF();
		delay_us(T55X7_BIT_GAP_US);
		FIELD_ON();
		
		data <<= 1;
	}
}

void rfid_t55x7_write_block(uint8_t *data, uint8_t address)
{
	rfid_t55x7_gap();
	rfid_t55x7_write_bits((T55X7_OPCODE_WRITE << 1) | T55X7_LATCH_UNLOCK, T55X7_OPCODE_BITS + T55X7_LATCH_BITS);
	for (uint8_t i = 0; i < T55X7_BLOCK_BYTES; i++)
	{
		rfid_t55x7_write_bits(*data++, 8);
	}
	rfid_t55x7_write_bits(address, T55X7_BLOCK_ADR_BITS);
	delay_ms(T55X7_BLOCK_WRITE_MS);
}

void rfid_t55x7_pack_key(uint8_t *packet, uint8_t *key)
{
	*packet++ = 0xFF;
	*packet   = 0x80;
	uint8_t packet_mask = BYTE_MSB >> 1;
	uint8_t key_mask    = BYTE_MSB;
	uint8_t col_parity  = 0x00;
	for (uint8_t i = 0; i < RFID_KEY_LENGTH*(8/ROW_BITS); i++)
	{
		uint8_t row_parity_bit = 0;
		for (uint8_t j = 0; j < ROW_BITS; j++)
		{
			if (*key & key_mask)
			{
				row_parity_bit ^= HIGH;
				*packet |= packet_mask;
			}
			
			packet_mask >>= 1;
			if (packet_mask == 0x00)
			{
				packet_mask = BYTE_MSB;
				*(++packet) = 0x00;
			}
			
			key_mask >>= 1;
			if (key_mask == 0x00)
			{
				key_mask = BYTE_MSB;
			}
		}
		if (row_parity_bit)
		{
			*packet |= packet_mask;
		}
		
		packet_mask >>= 1;
		if (packet_mask == 0x00)
		{
			packet_mask = BYTE_MSB;
			*(++packet) = 0x00;
		}
		
		if (i & 0x01)
		{
			col_parity ^= *key++ & 0x0F;
		}
		else
		{
			col_parity ^= *key >> ROW_BITS;
		}
	}
	*packet |= (col_parity << 1);
}

void rfid_t55x7_write_key(uint8_t *key)
{
	FIELD_ON();
	delay_ms(T55X7_WRITE_MS);
	
	static uint8_t config[T55X7_BLOCK_BYTES] =
	{
		(T55X7_CONFIG_KEY_WRITE >> 24) & 0xFF,
		(T55X7_CONFIG_KEY_WRITE >> 16) & 0xFF,
		(T55X7_CONFIG_KEY_WRITE >>  8) & 0xFF,
		(T55X7_CONFIG_KEY_WRITE >>  0) & 0xFF
	};
	
	static uint8_t packet[PACKET_BITS/8];
	rfid_t55x7_pack_key(&packet[0], key);
	
	rfid_t55x7_write_block(&packet[T55X7_BLOCK_BYTES*0], T55X7_BLOCK_ADR_KEY_H);
	rfid_t55x7_write_block(&packet[T55X7_BLOCK_BYTES*1], T55X7_BLOCK_ADR_KEY_L);
	rfid_t55x7_write_block(&config[0],                   T55X7_BLOCK_ADR_CONFIG);
	rfid_t55x7_gap();
	rfid_t55x7_write_bits(T55X7_OPCODE_RESET, T55X7_OPCODE_BITS);
	
	delay_ms(T55X7_WRITE_MS);
	FIELD_OFF();
}

void rfid_t55x7_write_keys(uint8_t *keys, uint8_t count)
{
	FIELD_ON();
	delay_ms(T55X7_WRITE_MS);
	
	static uint8_t config[T55X7_BLOCK_BYTES] =
	{
		((T55X7_CFG_BITRATE_RF64 | T55X7_CFG_MOD_MANCHESTER | T55X7_CFG_SEQ_TERM_ON) >> 24) & 0xFF,
		((T55X7_CFG_BITRATE_RF64 | T55X7_CFG_MOD_MANCHESTER | T55X7_CFG_SEQ_TERM_ON) >> 16) & 0xFF,
		((T55X7_CFG_BITRATE_RF64 | T55X7_CFG_MOD_MANCHESTER | T55X7_CFG_SEQ_TERM_ON) >>  8) & 0xFF,
		((T55X7_CFG_BITRATE_RF64 | T55X7_CFG_MOD_MANCHESTER | T55X7_CFG_SEQ_TERM_ON) >>  0) & 0xFF
	};
	config[3] |= (count & 0x03) << 6; 

	static uint8_t packet[PACKET_BITS/8*3];
	for (uint8_t i = 0; i < count; i++)
	{
		rfid_t55x7_pack_key(&packet[PACKET_BITS/8*i], keys+i*RFID_KEY_LENGTH);
	}
	
	for (uint8_t i = 0; i < count; i++)
	{
		rfid_t55x7_write_block(&packet[PACKET_BITS/8*i+T55X7_BLOCK_BYTES*0], (i<<1)+1);
		rfid_t55x7_write_block(&packet[PACKET_BITS/8*i+T55X7_BLOCK_BYTES*1], (i<<1)+2);
	}
	rfid_t55x7_write_block(&config[0], T55X7_BLOCK_ADR_CONFIG);
	rfid_t55x7_gap();
	rfid_t55x7_write_bits(T55X7_OPCODE_RESET, T55X7_OPCODE_BITS);
	
	delay_ms(T55X7_WRITE_MS);
	FIELD_OFF();
}


void rfid_em4x05_write_bit(uint8_t b)
{
	if (b)
	{
		FIELD_ON();
		delay_us(EM4X05_BIT_ONE_HIGH_US);
	}
	else
	{
		FIELD_OFF();
		delay_us(EM4X05_BIT_ZERO_LOW_US);
		FIELD_ON();
		delay_us(EM4X05_BIT_ZERO_HIGH_US);
	}
}

void rfid_em4x05_gap()
{
	FIELD_OFF();
	delay_us(EM4X05_GAP_LOW1_US);
	FIELD_ON();
	delay_us(EM4X05_GAP_HIGH1_US); 
	FIELD_OFF();
	delay_us(EM4X05_GAP_LOW2_US); 
	FIELD_ON();
	delay_us(EM4X05_GAP_HIGH2_US);
	
	rfid_em4x05_write_bit(LOW);
}

void rfid_em4x05_write_bits(uint8_t data, uint8_t bits)
{
	uint8_t parity = bits & EM4X05_CRC_ENABLE;
	bits &= ~EM4X05_CRC_ENABLE;

	while (bits--)
	{
		uint8_t b = data & BYTE_LSB ? HIGH : LOW;
		rfid_em4x05_write_bit(b);
		parity ^= b;
		data  >>= 1;
	}

	rfid_em4x05_write_bit(parity == (EM4X05_CRC_ENABLE | BYTE_LSB) ? HIGH : LOW);
}

void rfid_em4x05_write_block(uint8_t *data, uint8_t address)
{
	rfid_em4x05_gap();
	rfid_em4x05_write_bits(EM4X05_COMMAND_WRITE_WORD, EM4X05_COMMAND_BITS | EM4X05_CRC_ENABLE);
	rfid_em4x05_write_bits(address, EM4X05_ADDRESS_BITS | EM4X05_CRC_ENABLE);
	
	uint8_t col_parity = 0x00;
	for (uint8_t i = 0; i < EM4X05_BLOCK_BYTES; i++)
	{
		rfid_em4x05_write_bits(*data, 8 | EM4X05_CRC_ENABLE);
		col_parity ^= *data++;
	}
	rfid_em4x05_write_bits(col_parity, 8 | EM4X05_CRC_DISABLE);
	
	FIELD_ON();
	delay_ms(EM4X05_BLOCK_WRITE_MS);
}

void rfid_em4x05_write_key(uint8_t *key)
{
	static uint8_t config[EM4X05_BLOCK_BYTES] =
	{
		(EM4X05_CONFIG_KEY_WRITE >>  0) & 0xFF,
		(EM4X05_CONFIG_KEY_WRITE >>  8) & 0xFF,
		(EM4X05_CONFIG_KEY_WRITE >> 16) & 0xFF,
		(EM4X05_CONFIG_KEY_WRITE >> 24) & 0xFF
	};
	
	static uint8_t packet[PACKET_BITS/8];
	uint8_t *ppacket = &packet[0];
	*ppacket++ = 0xFF;
	*ppacket   = 0x01;
	uint8_t packet_mask = BYTE_LSB << 1;
	uint8_t key_mask    = BYTE_MSB;
	uint8_t col_parity  = 0x00;
	for (uint8_t i = 0; i < RFID_KEY_LENGTH*(8/ROW_BITS); i++)
	{
		uint8_t row_parity_bit = 0;
		for (uint8_t j = 0; j < ROW_BITS; j++)
		{
			if (*key & key_mask)
			{
				row_parity_bit ^= HIGH;
				*ppacket |= packet_mask;
			}
			
			packet_mask <<= 1;
			if (packet_mask == 0x00)
			{
				packet_mask = BYTE_LSB;
				*(++ppacket) = 0x00;
			}
			
			key_mask >>= 1;
			if (key_mask == 0x00)
			{
				key_mask = BYTE_MSB;
			}
		}
		if (row_parity_bit)
		{
			*ppacket |= packet_mask;
		}
		
		packet_mask <<= 1;
		if (packet_mask == 0x00)
		{
			packet_mask = BYTE_LSB;
			*(++ppacket) = 0x00;
		}
		
		if (i & 0x01)
		{
			col_parity ^= *key++ & 0x0F;
		}
		else
		{
			col_parity ^= *key >> ROW_BITS;
		}
	}
	for (uint8_t i = 0; i < ROW_BITS; i++)
	{
		if (col_parity & 0x08)
		{
			*ppacket |= packet_mask;
		}

		packet_mask <<= 1;
		col_parity  <<= 1;
	}

	FIELD_ON();
	delay_ms(EM4X05_WRITE_MS);
	
	rfid_em4x05_gap();
	rfid_em4x05_write_bits(EM4X05_COMMAND_LOGIN, EM4X05_COMMAND_BITS | EM4X05_CRC_ENABLE);
	for (uint8_t i = 0; i < EM4X05_BLOCK_BYTES; i++)
	{
		rfid_em4x05_write_bits(0xFF, 8 | EM4X05_CRC_ENABLE);
	}
	rfid_em4x05_write_bits(0x00, 8 | EM4X05_CRC_DISABLE);
	
	FIELD_ON();
	delay_ms(EM4X05_BLOCK_WRITE_MS);
	
	rfid_em4x05_write_block(&config[0],                  EM4X05_BLOCK_ADR_CONFIG);
	rfid_em4x05_write_block(&packet[0],                  EM4X05_BLOCK_ADR_KEY_L);
	rfid_em4x05_write_block(&packet[EM4X05_BLOCK_BYTES], EM4X05_BLOCK_ADR_KEY_H);
	
	FIELD_OFF();
	delay_ms(EM4X05_WRITE_MS);
}


void rfid_receive_key()
{
	if (rfid.packet_bit_counter < START_BITS)
	{
		if (rfid.current_bit == BIT_ZERO)
		{
			rfid.packet_bit_counter = PACKET_BITS_RESTART;
		}
	}
	else
	{
		rfid.parity_bit ^= rfid.current_bit;
		
		if (++rfid.row_bit_counter == ROW_PARITY_BIT)
		{
			if (((rfid.packet_bit_counter != STOP_BIT) && rfid.parity_bit) || ((rfid.packet_bit_counter == STOP_BIT) && (rfid.current_bit != BIT_ZERO)))
			{
				rfid.packet_bit_counter = PACKET_BITS_RESTART;
				rfid.row_counter        = 0;
			}
			else
			{
				rfid.receive_rows[rfid.row_counter] = rfid.receive_row;
				rfid.row_counter++;
			}
			
			rfid.row_bit_counter = 0;
			rfid.parity_bit      = LOW;
			rfid.receive_row     = 0x00;
		}
		else
		{
			rfid.receive_row <<= 1;
			if (rfid.current_bit)
			{
				rfid.receive_row |= BYTE_LSB;
			}
		}
	}
	
	if (++rfid.packet_bit_counter == PACKET_BITS)
	{
		CAPTURE_STOP();
		FIELD_OFF();
		CAPTURE_DISABLE();
		
		rfid.received_flag = TRUE;
	}
}

void rfid_receive_block()
{
	if (rfid.packet_bit_counter == 0)
	{
		if (rfid.current_bit == BIT_ONE)
		{
			rfid.packet_bit_counter = PACKET_BITS_RESTART;
		}
	}
	else
	{
		rfid.receive_row <<= 1;
		if (rfid.current_bit)
		{
			rfid.receive_row |= BYTE_LSB;
		}
		
		if (++rfid.row_bit_counter == 8)
		{
			rfid.receive_rows[rfid.row_counter] = rfid.receive_row;
			rfid.row_counter++;
			
			rfid.row_bit_counter = 0;
			rfid.receive_row     = 0x00;
		}
	}
	
	if (++rfid.packet_bit_counter == T55X7_BLOCK_BITS)
	{
		CAPTURE_STOP();
		CAPTURE_DISABLE();
		
		rfid.received_flag = TRUE;
	}
}

INT_HANDLER(IV_INT0_REQUEST) 
{
	uint16_t pulse_len;
	CAPTURE_RESTART(pulse_len);
	
	rfid.half_bit    = RFID_INPUT;
	rfid.current_bit = BIT_UNDEF;
		
	if (RFID_HBIT_MIN_US < pulse_len && pulse_len < RFID_HBIT_MAX_US)
	{
		if (++rfid.half_bit_counter == 2)
		{	
			rfid.current_bit      = rfid.half_bit;
			rfid.half_bit_counter = 0;
		}
		else if (rfid.half_bit_counter == 1)
		{
			rfid.current_bit = BIT_WAIT;
		}
	}
	else if (RFID_FBIT_MIN_US < pulse_len && pulse_len < RFID_FBIT_MAX_US)
	{
		if (++rfid.half_bit_counter == 2)
		{
			rfid.current_bit      = rfid.half_bit;
			rfid.half_bit_counter = 1;
		}
	}
		
	if (rfid.current_bit < BIT_UNDEF)
	{
		if (rfid.receive_mode == RECEIVE_MODE_KEY)
		{
			rfid_receive_key();
		}
		else
		{
			rfid_receive_block();
		}
	}
	else if (rfid.current_bit == BIT_UNDEF)
	{
		CAPTURE_RESET();
	}
		
	CAPTURE_CLEAR();
}

INT_HANDLER(IV_TMR1_OVERLOW)
{
	CAPTURE_STOP();
	CAPTURE_RESET();
}
