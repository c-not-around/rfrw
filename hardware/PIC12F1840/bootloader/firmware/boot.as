		processor	12F1840
		include		"12f1840.inc"
		
		global		_main
		global		start_initialization
		
		psect		config_words,class=CONFIG,delta=2
		DW			FCMEN_OFF & IESO_OFF & CLKOUTEN_OFF & BOREN_OFF & CPD_OFF & CP_OFF & MCLRE_OFF & PWRTE_ON & WDTE_OFF & FOSC_INTOSC
		DW			LVP_OFF & BORV_25 & STVREN_ON & PLLEN_ON & WRT_OFF
		
		psect		boot_code,class=CODE,delta=2
		
		
start_initialization:
		
		
; =====================================
;		Constants
; =====================================
XTAL_FREQ					EQU	32000000
BAUD_RATE					EQU	9600
UART_SPBRG					EQU	(XTAL_FREQ/(4*BAUD_RATE)-1)
; =====================================
WAIT_BYTE_CYCLES			EQU	250
WAIT_BYTE_STEP_US			EQU	40
; =====================================
INVALID_DATA_BYTE			EQU	0xFF
INVALID_DATA_WORD			EQU	0x8000
; =====================================
FLASH_BLOCK_SIZE			EQU	32
BOOT_VECTOR					EQU	0x0F00
; =====================================
HOST_RESP_SUCCESS			EQU	0x50
HOST_RESP_ERROR				EQU	0xE0
HOST_RESP_INVALID_ADR		EQU	0xA0
; =====================================
CMD_FLASH_READ_BLOCK		EQU	0x10
CMD_FLASH_FILL_BLOCK		EQU	0x20
CMD_FLASH_ERASE_ROW			EQU	0x30
CMD_ECHO_PRESENCE			EQU	0x40
CMD_LOADING_END				EQU	0x80
; =====================================
NIBBLE_LOW					EQU	0x0F
NIBBLE_HIGH					EQU	0xF0
; =====================================
EECON2_WHITE_MAGIC			EQU	0x55
EECON2_BLACK_MAGIC			EQU	0xAA
; =====================================
;		Variables
; =====================================
delay_40us_count1			EQU 0x71
delay_40us_count2			EQU 0x7B
; =====================================
uart_read_byte_timeout		EQU	0x70
; =====================================
host_write_byte_data		EQU	0x70
; =====================================
host_read_word_ret_ll		EQU	0x72
host_read_word_ret_hl		EQU	0x73
host_read_word_temp_lh		EQU	0x74
host_read_word_temp_hh		EQU	0x75
; =====================================
main_command				EQU	0x76
main_address_l				EQU	0x77
main_address_h				EQU	0x78
main_length_l				EQU	0x79
main_length_h				EQU	0x7A
		
		
_main:
		; =====================================
		;		Initialization
		; =====================================
		MOVLB	BANK1
		MOVLW	11110000B
		MOVWF	OSCCON
		CLRF	OPTION_REG
		MOVLW	11111011B
		MOVWF	TRISA
		MOVLB	BANK2
		CLRF	CM1CON0
		CLRF	FVRCON
		MOVLW	00000001B
		MOVWF	APFCON
		MOVLB	BANK4
		MOVLW	00001000B
		MOVWF	WPUA
		MOVLB	BANK0
		CLRF	INTCON
		CLRF	CPSCON0
		CLRF	PORTA
		MOVLB	BANK3
		CLRF	ANSELA
		; =====================================
		;		UART Initialization
		; =====================================
		MOVLW	(1 << SPEN) | (1 << CREN)
		MOVWF	RCSTA
		MOVLW	(1 << TXEN) | (1 << BRGH)
		MOVWF	TXSTA
		MOVLW	(1 << BRG16)
		MOVWF	BAUDCON
		MOVLW	LOW(UART_SPBRG)
		MOVWF	SPBRGL
		MOVLW	HIGH(UART_SPBRG)
		MOVWF	SPBRGH
main_loop:
		; =====================================
		;		Main Loop
		; =====================================
main_receive_wait:
		MOVLB	BANK0
		BTFSS	PIR1, RCIF
		LJMP	main_receive_wait
		MOVLB	BANK3
		MOVF	RCREG, W
		MOVWF	main_command
		; =====================================
		;		Command Echo Presence
		; =====================================
		XORLW	CMD_ECHO_PRESENCE
		BTFSC	STATUS, ZERO
		LJMP	main_send_success
		; =====================================
		;		Command Load Release
		; =====================================
		MOVF	main_command, W
		XORLW	CMD_LOADING_END
		BTFSS	STATUS, ZERO
		LJMP	main_if_param_cmd
		FCALL	_host_send_ack
main_send_wait:
		MOVLB	BANK0
		BTFSS	PIR1, TXIF
		LJMP	main_send_wait
		MOVLW	250
		FCALL	_delay_40us
		RESET
main_if_param_cmd:
		; =====================================
		;		Commands with parameters
		; =====================================
		MOVF	main_command, W
		ANDLW	CMD_FLASH_READ_BLOCK | CMD_FLASH_FILL_BLOCK | CMD_FLASH_ERASE_ROW
		BTFSC	STATUS, ZERO
		LJMP	main_loop
		; =====================================
		;		Receive paramters (2 words) from host
		; =====================================
		FCALL	_host_read_word
		BTFSC	host_read_word_ret_hl, 7
		LJMP	main_loop
		MOVF	host_read_word_ret_ll, W
		MOVWF	main_address_l
		MOVF	host_read_word_ret_hl, W
		MOVWF	main_address_h
		FCALL	_host_read_word
		BTFSC	host_read_word_ret_hl, 7
		LJMP	main_loop
		MOVF	host_read_word_ret_ll, W
		MOVWF	main_length_l
		MOVF	host_read_word_ret_hl, W
		MOVWF	main_length_h
		; =====================================
		;		Command Flash Read
		; =====================================
		MOVF	main_command, W
		XORLW	CMD_FLASH_READ_BLOCK
		BTFSS	STATUS, ZERO
		LJMP	main_if_next_cmd
		MOVLW	(1 << EEPGD) | (0 << CFGS)
		MOVWF	EECON1
main_flash_read_cycle:
		MOVF	main_address_l, W
		MOVWF	EEADRL
		MOVF	main_address_h, W
		MOVWF	EEADRH
		INCF	main_address_l, F
		BTFSC	STATUS, ZERO
		INCF	main_address_h, F
		BSF		EECON1, RD
		NOP
		NOP
		MOVF	EEDATL, W
		FCALL	_host_write_byte
		MOVF	EEDATH, W
		FCALL	_host_write_byte
		DECFSZ	main_length_l, F
		LJMP	main_flash_read_cycle
main_if_next_cmd:
		; =====================================
		;		Command Flash Write
		; =====================================
		MOVF	main_command, W
		XORLW	CMD_FLASH_FILL_BLOCK
		BTFSS	STATUS, ZERO
		LJMP	main_if_next_cmd_2
		MOVLW	FLASH_BLOCK_SIZE + 1
		SUBWF	main_length_l, W
		BTFSC	STATUS, CARRY
		LJMP	main_invalid_address
		MOVLW	HIGH(BOOT_VECTOR - FLASH_BLOCK_SIZE + 1)
		SUBWF	main_address_h, W
		MOVLW	LOW(BOOT_VECTOR - FLASH_BLOCK_SIZE + 1)
		BTFSC	STATUS, ZERO
		SUBWF	main_address_l, W
		BTFSC	STATUS, CARRY
		LJMP	main_invalid_address
		FCALL	_host_send_ack
		MOVF	main_address_h, W
		MOVWF	EEADRH
		MOVF	main_address_l, W
		MOVWF	EEADRL
		MOVLW	(1 << EEPGD) | (0 << CFGS) | (1 << LWLO) | (0 << FREE) | (1 << WREN)
		MOVWF	EECON1
main_flash_fill_cycle:
		FCALL	_host_read_word
		BTFSC	host_read_word_ret_hl, 7
		LJMP	main_loop
		FCALL	_host_send_ack
		MOVF	host_read_word_ret_ll, W
		MOVWF	EEDATL
		MOVF	host_read_word_ret_hl, W
		MOVWF	EEDATH
		MOVF	main_length_l, W
		XORLW	0x01
		BTFSC	STATUS, ZERO
		BCF		EECON1, LWLO
		FCALL	_flash_write_sequence
		INCF	EEADRL, F
		DECFSZ	main_length_l, F
		LJMP	main_flash_fill_cycle
		BCF		EECON1, WREN
		LJMP	main_send_success
main_if_next_cmd_2:
		; =====================================
		;		Command Flash Erase
		; =====================================
		MOVF	main_command, W
		XORLW	CMD_FLASH_ERASE_ROW
		BTFSS	STATUS, ZERO
		LJMP	main_loop
		MOVLW	HIGH(BOOT_VECTOR - FLASH_BLOCK_SIZE + 1)
		SUBWF	main_address_h, W
		MOVLW	LOW(BOOT_VECTOR - FLASH_BLOCK_SIZE + 1)
		BTFSC	STATUS, ZERO
		SUBWF	main_address_l, W
		BTFSC	STATUS, CARRY
		LJMP	main_invalid_address
		MOVF	main_address_l, W
		MOVWF	EEADRL
		MOVF	main_address_h, W
		MOVWF	EEADRH
		MOVLW	(1 << EEPGD) | (0 << CFGS) | (1 << FREE) | (1 << WREN)
		MOVWF	EECON1
		FCALL	_flash_write_sequence
		BCF		EECON1, WREN
		LJMP	main_send_success
		; =====================================
		;		End of Main Loop
		; =====================================
		LJMP	main_loop
		; =====================================
		;		Send Success and Repeat
		; =====================================
main_send_success:
		FCALL	_host_send_ack
		LJMP	main_loop
		; =====================================
		;		Send Error and Repeat
		; =====================================
main_send_error:
		MOVLW	HOST_RESP_ERROR
		FCALL	_host_write_byte
		LJMP	main_loop
		; =====================================
		;		Send Invalid Address and Repeat
		; =====================================
main_invalid_address:
		MOVLW	HOST_RESP_INVALID_ADR
		FCALL	_host_write_byte
		LJMP	main_loop
		
		
		
		; =====================================
		;		Delay 1+324t/8 us
		; =====================================
_delay_40us:
		MOVWF	delay_40us_count1
delay_40us_cycle1:
		MOVLW	106
		MOVWF	delay_40us_count2
delay_40us_cycle2:
		DECFSZ	delay_40us_count2, F
		LJMP	delay_40us_cycle2
		DECFSZ	delay_40us_count1, F
		LJMP	delay_40us_cycle1
		RETURN
		
		
		; =====================================
		;		Uart Receive byte
		; =====================================
_uart_read_byte:
		MOVLW	WAIT_BYTE_CYCLES
		MOVWF	uart_read_byte_timeout
uart_read_byte_cycle:
		MOVLB	BANK0
		BTFSS	PIR1, RCIF
		LJMP	uart_read_byte_delay
		MOVLB	BANK3
		MOVF	RCREG, W
		RETURN
uart_read_byte_delay:
		MOVLW	1
		FCALL	_delay_40us
		DECFSZ	uart_read_byte_timeout
		LJMP	uart_read_byte_cycle
		RETLW	INVALID_DATA_BYTE
		
		
		; =====================================
		;		Uart Transmit byte
		; =====================================
_uart_write_byte:
		MOVLB	BANK0
		BTFSS	PIR1, TXIF
		LJMP	_uart_write_byte
		MOVLB	BANK3
		MOVWF	TXREG
		RETURN
		
		
		; =====================================
		;		Send Acknowledge to Host
		; =====================================
_host_send_ack:
		MOVLW	HOST_RESP_SUCCESS
		; =====================================
		;		Send byte to Host
		; =====================================
_host_write_byte:
		MOVWF	host_write_byte_data
		ANDLW	NIBBLE_LOW
		FCALL	_uart_write_byte
		SWAPF	host_write_byte_data, W
		ANDLW	NIBBLE_LOW
		LJMP	_uart_write_byte
		
		
		; =====================================
		;		Get word from Host
		; =====================================
_host_read_word:
		FCALL	_uart_read_byte
		MOVWF	host_read_word_ret_ll
		FCALL	_uart_read_byte
		MOVWF	host_read_word_temp_lh
		FCALL	_uart_read_byte
		MOVWF	host_read_word_ret_hl
		FCALL	_uart_read_byte
		MOVWF	host_read_word_temp_hh
		; =====================================
		MOVF	host_read_word_ret_ll
		IORWF	host_read_word_temp_lh, W
		IORWF	host_read_word_ret_hl, W
		IORWF	host_read_word_temp_hh, W
		ANDLW	NIBBLE_HIGH
		BTFSS	STATUS, ZERO
		LJMP	host_read_word_fail
		;======================================
		SWAPF	host_read_word_temp_lh, W
		IORWF	host_read_word_ret_ll, F
		SWAPF	host_read_word_temp_hh, W
		IORWF	host_read_word_ret_hl, F
		RETURN
host_read_word_fail:
		MOVLW	HOST_RESP_ERROR
		FCALL	_host_write_byte
		BSF		host_read_word_ret_hl, 7
		RETURN
		
		
		; =====================================
		;		Flash Write Unlock Sequence
		; =====================================
_flash_write_sequence:
		MOVLW	EECON2_WHITE_MAGIC
		MOVWF	EECON2
		MOVLW	EECON2_BLACK_MAGIC
		MOVWF	EECON2
		BSF		EECON1, WR
		NOP
		NOP
flash_write_poll:
		BTFSC	EECON1, WR
		LJMP	flash_write_poll
		RETURN