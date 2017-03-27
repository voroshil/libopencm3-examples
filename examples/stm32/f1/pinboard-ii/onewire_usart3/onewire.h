#ifndef _H_ONEWIRE_H_
#define _H_ONEWIRE_H_

typedef enum {OK=0, NO_SENSORS, MISSING_SENSOR, LOW_TEMP, HIGH_TEMP} state_t;

#define OW_FAMILY_DS18B20 0x28
#define OW_FAMILY_DS18S20 0x10

/*
 * thermometer commands
 * send them with bus reset!
 */
// find devices
#define OW_SEARCH_ROM       (0xf0)
// read device (when it is alone on the bus)
#define OW_READ_ROM         (0x33)
// send device ID (after this command - 8 bytes of ID)
#define OW_MATCH_ROM        (0x55)
// broadcast command
#define OW_SKIP_ROM         (0xcc)
// find devices with critical conditions
#define OW_ALARM_SEARCH     (0xec)
/*
 * thermometer functions
 * send them without bus reset!
 */
// start themperature reading
#define OW_CONVERT_T         (0x44)
// write critical temperature to device's RAM
#define OW_SCRATCHPAD        (0x4e)
// read whole device flash
#define OW_READ_SCRATCHPAD   (0xbe)
// copy critical themperature from device's RAM to its EEPROM
#define OW_COPY_SCRATCHPAD   (0x48)
// copy critical themperature from EEPROM to RAM (when power on this operation runs automatically)
#define OW_RECALL_E2         (0xb8)
// check whether there is devices wich power up from bus
#define OW_READ_POWER_SUPPLY (0xb4)

/*
 * RAM register:
 * 0 - themperature LSB
 * 1 - themperature MSB (all higher bits are sign)
 * 2 - T_H
 * 3 - T_L
 * 4 - B20: Configuration register (only bits 6/5 valid: 9..12 bits resolution); 0xff for S20
 * 5 - 0xff (reserved)
 * 6 - (reserved for B20); S20: COUNT_REMAIN (0x0c)
 * 7 - COUNT PER DEGR (0x10)
 * 8 - CRC
 *
 * To identify S20/B20 use value of confuguration register: its MSbit is 0
 */

typedef enum{
	OW_MODE_OFF,        // sleeping
	OW_MODE_TRANSMIT_N, // transmit N bytes
	OW_MODE_RECEIVE_N,  // receive N bytes
	OW_MODE_RESET       // reset bus
} OW_modes;

extern uint8_t ROM[];


void onewire_init();
void onewire_handler();
void onewire_temp_handler();
void onewire_scan_handler();
uint8_t onewire_reset();

#endif  // _H_ONEWIRE_H_
