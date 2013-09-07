#include "command.h"

#include "config.h"
#include "debug.h"
#include "pwm.h"
#include "usart2.h"

/*
 * The commands that may arrive on the USART.
 */
typedef enum {
	// normal mode
	CMD_BROADCAST = 0xfd,
	CMD_STROBE = 0xfe,
} command_t;

/*
 * Returns the length of data (including the command) to be
 * expected for a given command code. If the command code is invalid,
 * 0 is returned.
 *
 * set_mode must be called before this.
 */
int command_length(char command_code) {
	// All commands have module length + 1,
	// Strobe has length 1.
	if (command_code == CMD_STROBE) {
		return 1;
	} else {
		return MODULE_LENGTH + 1;
	}
}

/*
 * The USART filter function.
 *
 * This function accepts the address of the module, the broadcast
 * address and all special commands.
 */
static uint8_t command_filter(uint8_t command) {
#ifdef TRACE_USART
	debug_string("t");
	debug_putchar(config.my_address);
#endif

	return command == config.my_address ||
		command == CMD_BROADCAST ||
		command == CMD_STROBE;
}

/*
 * Initializes the command module and sets up the USART filter
 * appropriately.
 */
void command_init() {
	usart2_set_filter(command_filter);
}

/*
 * Runs a "set LEDs" command. The expected format is:
 * <Module address> <LED0 brightness> <LED1 brightness> ...
 */
static error_t run_set_leds(uint8_t *led_values, uint8_t length) {
#ifdef TRACE_COMMANDS
	debug_string("set");
	debug_putchar(length);
	debug_write((char*) led_values, length);
#endif

	error_t error;

	if (MODULE_LENGTH < length) {
		length = MODULE_LENGTH;
	}

	for (uint8_t i = 0; i < length; i++) {
		error = pwm_set_brightness(i, led_values[i] << 8);

		if (error) return error;
	}

	return E_SUCCESS;
}
/*
 * Runs the command pointed to by 'command'.
 *
 * Returns an error/success code.
 */
error_t run_command(uint8_t *command) {
#ifdef TRACE_COMMANDS
	debug_putchar(command[0]);
#endif

	if (command[0] == config.my_address ||
	    command[0] == CMD_BROADCAST) {
		// The first byte is the addresss. Drop it.
		return run_set_leds(command + 1, MODULE_LENGTH);
	} else if (command[0] == CMD_STROBE) {
		return pwm_send_frame();
	}

	// else
	return E_WRONGCOMMAND;
}
