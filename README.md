# RFRW

USB RFID 125kHz reader and programmer

## Supported types of contactless identification devices

EM4100 compatible devices are supported:

  * T5557
  * T5567
  * T5577
  * EM4205
  * EM4305

## Hardware

  The project is implemented in the form of several platforms, based on the ATmega328P and PIC12F1840 microcontrollers.

  * The default platform is a PIC12F1840-based USB reader/programmer.
  * Embedded module based on PIC12F1840.
  * Modified RDM6300 module (the standard controller has been replaced with a PIC12F1840).
  * USB reader/programmer based on Arduino UNO / Nano.
  * Embedded module based on ATmega328P.

The analog part is based on the circuit implemented in the RDM6300 reader. This circuit has one input and one output. A square wave with the RFID frequency `125kHz` must be fed to the input. The output of the circuit will be the detected and amplified pulses from the read tag. This circuit, implemented as a module, can be connected to almost any microcontroller. To work with such RFID modules, the microcontroller must have the following peripheral devices:
  * PWM / Timer module or other module capable of outputting a `125kHz` square wave to some GPIO output;
  * External interrupt on level change (rising and falling edges) on one of the pins;
  * A timer capable of counting time intervals of about `1000` microseconds with an accuracy of at least `1us`.

The firmware is implemented for two types of microcontrollers: (PIC and AVR) and can be ported to other microcontrollers, both within these families and to completely different ones.

## Firmware

  The following tasks are implemented in the firmware:

  * Continuous reading of tag identifiers within the device's field of action.
  * Verification of codes obtained by reading tags and, if the value is reliable, placing this data in a queue.
  * Servicing the read data queue.
  * Writing a key to a T55x7 device. Converting a 5-byte key to a format supported by the T55x7.
  * Writing a key to a EM4x05 device. Converting a 5-byte key to a format supported by the EM4x05.
  * Execution of commands received via the UART interface in accordance with the protocol.

### Protocol

  The protocol uses a master-slave model. The master is the host (software on the PC). A command consists of a single byte identifier and optionally a number of parameters. In response to host commands, the device may respond with return codes or the requested data.
  
  Command set:
  
  | Mnemonic               | Command | 
  | :--------------------- | :-----: |
  | `CMD_GET_COUNT_KEY`    | `0x04`  |
  | `CMD_GET_NEXT_KEY`     | `0x08`  |
  | `CMD_RESET_QUEUE`      | `0x10`  |
  | `CMD_WRITE_T55X7`      | `0x20`  |
  | `CMD_WRITE_EM4X05`     | `0x40`  |
  | `CMD_START_BOOTLOADER` | `0x02`  |
  
  Return codes:
  
  | Mnemonic               |  Code  | 
  | :--------------------- | :----: |
  | `RESP_SUCCESS`         | `0x00` |
  | `RESP_INVALID_COMMAND` | `0x81` |
  | `RESP_KEY_QUEUE_EMPTY` | `0x82` |
  | `RESP_INVALID_LENGTH`  | `0x83` |
  | `RESP_INVALID_VALUE`   | `0x84` |
  | `RESP_INVALID_PASS`    | `0x85` |

  The data is encoded using the tetrad-byte method, offset by `0x47` (or character code `G`). The encoded data packet ends with an 8-bit checksum, encoded in the same way. The checksum is the zero's complement of the sum of all the original data. Therefore, the sum of the decoded data of a valid packet (all bytes including the checksum byte) must equal zero. For example:
  
  |                                                            | Description          |
  | :--------------------------------------------------------- | :------------------- |
  | `0x12`, `0x34`                                             | Initial data         |
  | `0x47 + 0x01`, `0x47 + 0x02`, `0x47 + 0x03`, `0x47 + 0x04` | Divide into tetrads  |
  | `0x48`, `0x49`, `0x4A`, `0x4B`                             | and add offset       |
  | `0x12` + `0x34` = `0x36`; `0xFF` - `0x36` + `1` = `0xCA`   | Checksum calculation |
  | `0x48`, `0x49`, `0x4A`, `0x4B`, `0x53`, `0x51`             | The final result     |
  | `H`, `I`, `J`, `K`, `S`, `Q`                               | ASCII Interpretation |
  
  * Command Get keys count `CMD_GET_COUNT_KEY`
    Response - A single byte containing the number of keys currently in the queue. Transmitted twice: in direct and inverse form.
  * Command Get next key value from queue `CMD_GET_NEXT_KEY`
    Sends 5 bytes of key as a packet encoded according to the rules described above. Waits for the host to confirm the packet's acceptance and correctness within a specified time. As confirmation, the host sends a checksum of the packet it received. If the host confirms the packet's reception, the key is removed from the queue; otherwise, it is considered unread and is removed from the queue.
  * Command Reset received keys queue `CMD_RESET_QUEUE`
    Removes all queued data, restarts continuous reading, and returns a no-error code `RESP_SUCCESS` to the host.
  * Command Write key to T55x7 compatible device `CMD_WRITE_T55X7`
    After receiving this command, the device waits for a specified time for a packet containing the value of the key(keys[^1]). If the length or content of the received packet is incorrect, a corresponding error code is sent (`RESP_INVALID_LENGTH`, `RESP_INVALID_VALUE`). If the key data is successfully received, it will be written to the label. Once the writing is complete, the `RESP_SUCCESS` code will be sent to the host and the continuous reading process will be restarted, losing the data currently contained in the queue.
  * Command Write key to EM4x05 compatible device `CMD_WRITE_EM4X05`
    After receiving this command, the device waits for a specified time for a packet containing the value of the key. If the length or content of the received packet is incorrect, a corresponding error code is sent (`RESP_INVALID_LENGTH`, `RESP_INVALID_VALUE`). If the key data is successfully received, it will be written to the label. Once the writing is complete, the `RESP_SUCCESS` code will be sent to the host and the continuous reading process will be restarted, losing the data currently contained in the queue.
  * Command Launch the bootloader `CMD_START_BOOTLOADER` (Default platform only)
    After receiving this command, the device waits for confirmation from the host within the specified time. As confirmation, the host must send a two-byte "password" 
	`CMD_BOOTLOADER_PASS` `0x5AA5`
	If the "password" is not received or is received but incorrect, the device sends an error code to the host `RESP_INVALID_PASS`. Otherwise, the device sends confirmation code `RESP_SUCCESS` to the host and, after some delay, proceeds to bootloader execution.
	
	[^1]: Note1: The memory structure and operating algorithm of T55x7 devices allow for the recording and use of up to three keys sequentially located in memory (and therefore transmitted sequentially). This capability has been implemented, and the write function, like the `CMD_WRITE_T55X7` command in general, accepts an array of keys as arguments. In this case, the number of keys is encoded in the two least significant bits of the `CMD_WRITE_T55X7` command.
  
### Bootloader

  A bootloader has been implemented for updating firmware for PIC12F1840-based platforms. The bootloader takes up `256` words of the PIC12F1840's `4096` flash memory, which is `6.25%`. The bootloader is activated in two possible ways:
  * by supplying power with the `RA3` pin shorted to ground with a jumper;
  * on command `CMD_START_BOOTLOADER`, proceed to executing the bootloader from the host.

The bootloader allows:
  * read the current firmware in its entirety or in separate blocks;
  * write blocks of the entire microcontroller memory, with the exception of the bootloader area itself (the last 256 words of program flash memory);
  * programmatically reboot the device after the download of the new version of the target firmware is complete and thus proceed to its execution.

To work with the bootloader, use the command line utility `picboot`. Usage:
  * Backup current target firmware:
    ``` picboot.exe -p=COM5 -d=9600 -b=32 -c=4096 -s=0xF00 -k=backup.hex ``` 
  * Downloading a new target firmware:
    ``` picboot.exe -p=COM5 -d=9600 -b=32 -c=4096 -s=0xF00 -f=project.hex ```

## Software

The host program on the PC allows you to: 
  * receive read tag keys from the device in continuous reading mode;
  * flash up to three keys into T55x7 tags;
  * flash up to a key into EM4x05 tags.

## Development tools

  | Task                         |  Tool                      | 
  | :--------------------------- | :------------------------- |
  | Schematics design            | Splan 8.0                  |
  | PCB design                   | SprintLayout 6.0           |
  | AVR Compiler                 | AVR GCC 5.4.0              |
  | Arduino sketch               | Arduino IDE 1.8.19         |
  | PIC Mid-Range Compiler       | HI-TECH PICC 9.71a         |
  | Make utility                 | GNU Make 4.1 for Windows32 |
  | Electronic circuit simulator | Proteus 8.13 SP0           |
  | CLI & GUI app for Windows    | PascalABC.NET 3.10.3       |