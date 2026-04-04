#ifndef _RFID_H
#define _RFID_H


#define RFID_KEY_LENGTH		5

#define KEY_STATUS_VALID	0x00
#define KEY_STATUS_WAIT		0x10
#define KEY_STATUS_BAD_CRC	0x20

#define WRITE_KEYS_MAX		3


void rfid_init();

void rfid_read_start();
uint8_t rfid_read_key(uint8_t *key);

void rfid_t55x7_write_key(uint8_t *key);
void rfid_t55x7_write_keys(uint8_t *keys, uint8_t count);
void rfid_em4x05_write_key(uint8_t *key);


#endif