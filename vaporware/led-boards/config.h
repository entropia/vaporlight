#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "stm_include/stm32/timer.h"

#include "error.h"
#include "led.h"

/*
 * Configuration to be set at compile time.
 */

// Number of milliseconds to wait between switching of the debug LED.
#define DEBUG_LED_SPEED 125 // 1/8 seconds per dot.

// Length of this module in LEDs
#define MODULE_LENGTH 16

// Mapping from timer channels to PWM registers.
static volatile uint32_t * const TIMER_CHANNELS[MODULE_LENGTH] = {
	&TR(TIM2,  CCR1),
	&TR(TIM2,  CCR2),
	&TR(TIM1,  CCR1),
	&TR(TIM1,  CCR2),
	&TR(TIM1,  CCR3),
	&TR(TIM2,  CCR3),
	&TR(TIM2,  CCR4),
	&TR(TIM15, CCR1),
	&TR(TIM15, CCR2),
	&TR(TIM3,  CCR1),
	&TR(TIM3,  CCR2),
	&TR(TIM3,  CCR3),
	&TR(TIM3,  CCR4),
	&TR(TIM1,  CCR4),
	&TR(TIM16, CCR1),
	&TR(TIM17, CCR1)
};

// Baud rate register for the USARTs.
// Divider 3.0 * 16 = 48.0: 500000 baud at 24MHz
static const int USART_BAUD_VALUE = (3 << 4) | 0;
// Divider 13.0 * 16 = 208.3: 115200 baud at 24MHz
static const int CONSOLE_BAUD_VALUE = (13 << 4) | 0;

// Timeout for the user reaction to enter config mode, in ms.
static const int ASK_MODE_TIMEOUT = 3000;

// Number of failures on which to raise an error.
static const int USART_FAIL_TRESHOLD = 20;

// Number of USART command buffers.
#define USART_BUFFER_COUNT 4
// Length of a USART command buffer
#define CMD_BUFFER_LEN 20

// Start-of-command marker
#define START_MARK 0x55
// Byte used for escaping
#define ESCAPE_MARK 0x54

// Temperature sensor count
#define HEAT_SENSOR_LEN 6


// Number base used for reading user input in the config console.
#define CONSOLE_READ_BASE 16


// Temperature sendor ADC ports
static const uint8_t HEAT_ADC_PORTS[HEAT_SENSOR_LEN] = {
	4,
	5,
	10,
	11,
	12,
	13,
};

// How many bits to use for the PWM.  This is now imported from the
// Makefile in order to synchronize with make_gamma_table.py.
// #define PWM_BITS 13

// The value to which the PWM counters will be reset.
#if PWM_BITS == 16
	// Use 0xfffe so that a value of 0xffff is really 100% on
	#define PWM_RELOAD 0xfffe
#else
	#define PWM_RELOAD ((1 << PWM_BITS) - 1)
#endif

// Sample time for the heat sensors.
#define ADC_SAMPLE_TIME 0x7 // 239.5 cycles (50kHz)
static const int ADC_SAMPLE_TIME_1 =
	((ADC_SAMPLE_TIME & 0x7) << 0) |
	((ADC_SAMPLE_TIME & 0x7) << 3) |
	((ADC_SAMPLE_TIME & 0x7) << 6) |
	((ADC_SAMPLE_TIME & 0x7) << 9) |
	((ADC_SAMPLE_TIME & 0x7) << 12) |
	((ADC_SAMPLE_TIME & 0x7) << 15) |
	((ADC_SAMPLE_TIME & 0x7) << 18) |
	((ADC_SAMPLE_TIME & 0x7) << 21);

static const int ADC_SAMPLE_TIME_2 =
	((ADC_SAMPLE_TIME & 0x7) << 0) |
	((ADC_SAMPLE_TIME & 0x7) << 3) |
	((ADC_SAMPLE_TIME & 0x7) << 6) |
	((ADC_SAMPLE_TIME & 0x7) << 9) |
	((ADC_SAMPLE_TIME & 0x7) << 12) |
	((ADC_SAMPLE_TIME & 0x7) << 15) |
	((ADC_SAMPLE_TIME & 0x7) << 18) |
	((ADC_SAMPLE_TIME & 0x7) << 21) |
	((ADC_SAMPLE_TIME & 0x7) << 24) |
	((ADC_SAMPLE_TIME & 0x7) << 27);

// Number of failures on which to raise an error.
static const int HEAT_FAIL_TRESHOLD = 20;

// Initial contents of Flash.
// This should be the value that the Flash contains when not programmed.
static const uint8_t EEPROM_EMPTY = 0xFF;

// Size of the config page in flash pages. When changing, the linker script
// must be edited accordingly.
#define CONFIG_PAGES 1

// Values of configuration entry status.
static const uint16_t CONFIG_ENTRY_EMPTY = 0xffff; // Should be default Flash value.
static const uint16_t CONFIG_ENTRY_IN_USE = 0x5555;
static const uint16_t CONFIG_ENTRY_OLD = 0x0000;

/*
 * Configuration to be read from Flash.
 */

/*
 * Struct for a complete set of configuration.
 */
typedef struct {
	// The whitepoint correction for each LED in this module.
	// Maps from physical LED to whitepoint correction.
	uint16_t white_correction[MODULE_LENGTH];

	// This module's address.
	// It is stored as a 16-bit integer so that the whole struct has a size
	// divisible by sizeof(uint16_t).
	uint16_t my_address;

	// Temperature limits for the heat sensors.
	uint16_t heat_limit[HEAT_SENSOR_LEN];

	// LED permutation. Maps from logical LED to physical LED.
	uint8_t physical_led[MODULE_LENGTH];

	// LED colors. Maps from physical LED to color.
	color_t led_color[MODULE_LENGTH];

} __attribute__ ((packed)) config_entry_t;

/*
 * Configuration values in RAM for access by other modules.
 */
extern config_entry_t config;

/*
 * Loads the configuration stored in flash. If no configuration is found,
 * E_NOCONFIG is returned.
 *
 * Returns an error/success code.
 */
error_t load_config();

/*
 * Saves the configuration to flash.
 */
error_t save_config();

/*
 * Checks if the configuration in config is valid.  Returns 1 if the
 * configuration is valid, 0 otherwise.  This function may print an
 * explanatory message to the debug USART if the configuration is
 * found to be invalid.
 */
int config_valid();

#endif
