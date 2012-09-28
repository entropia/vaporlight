#ifndef ERROR_H
#define ERROR_H

#include <string.h>

/*
 * Provides some functions for error reporting.
 */

/*
 * All the error conditions that appear throughout the program.
 */
typedef enum {
	E_SUCCESS = 0,
	E_INDEXRANGE, // An index was out of range.
	E_WRONGCOMMAND, // An undefined command was sent.
	E_NOCONFIG, // No configuration was found in EEPROM.
	E_FLASH_WRITE, // An error ocurred while writing to flash.
	E_MISSING_ARGS, // A console command was missing arguments.
	E_ARG_FORMAT, // An argument on the console was malformed
		      // (i.e. not an integer) or out of range.
} error_t;

/*
 * The possible reasons for an error. These are used bitwise as debug LED
 * blinking patterns, where a set bit means the LED is on and an unset bit
 * means it is off.
 */
typedef enum {
	// "NC" No configuration has been found
	ER_NO_CONFIG   = 0b01110100011101011101000000000000,
	// "BUG" Generic bug error.
	ER_BUG         = 0b01101010100010101100011101110100,
	// "HEAT" Overheat has been detected
	ER_HEAT        = 0b01010101000100010111000111000000,
	// "RX" Error in USART reception
	ER_USART_RX    = 0b01011101000111010101110000000000,
	// "FW" An error ocurred while writing to flash.
	ER_FLASH_WRITE = 0b01010111010001011101110000000000,
	// "CO" Too many commands are queued in the USART buffers
	ER_CMDOVERFLOW = 0b01110101110100011101110111000000
} err_reason_t;

/*
 * Specifies how to continue after an error has occurred.
 */
typedef enum {
	EA_PANIC,    // Get stuck in an infinite loop after the error.
	EA_RESET,    // Reset the device after an error.
	EA_RESUME,   // Resume normally after an error.
} err_action_t;

/*
 * Reports an error. reason should roughly give the reason for the error. Further
 * details can be supplied in the message (which may contain null bytes and whose
 * length is indicated by length). action describes which actions to take after the
 * error has been reported.
 */
void error(err_reason_t reason, char *message, int length, err_action_t action);

/*
 * Switches on the debug LED.
 */
void dled_on();

/*
 * Switches off the debug LED.
 */
void dled_off();

/*
 * Toggles the debug LED.
 */
void dled_toggle();

/*
 * A little macro to make using error more convenient.
 */
#define STR_WITH_LEN(str) str, strlen(str)

#endif
