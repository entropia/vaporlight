#include "command.h"

#include "color.h"
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
 * The options that can be sent in the first byte of a "set LEDs" command.
 */
typedef enum {
	OPT_CORRECTION = 0x01,
} set_led_options_t;

/*
 * Returns the length of data (including the command) to be
 * expected for a given command code. If the command code is invalid,
 * 0 is returned.
 *
 * set_mode must be called before this.
 */
int command_length(char command_code) {
	// The "set LEDs" commands have the format:
	// address options (channels * MODULE_LENGTH)
	// 1 byte  1 byte   2 bytes each

	// The "strobe" command stands for itself
	if (command_code == CMD_STROBE) {
		return 1;
	} else {
		return 2 * MODULE_LENGTH + 2;
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
static error_t run_set_leds(uint8_t *args) {
#ifdef TRACE_COMMANDS
	debug_string("set");
	debug_putchar(length);
	debug_write((char*) args, 2 * MODULE_LENGTH + 2);
#endif

	error_t error;
	int i = 0;

	int options = args[i++];

	if (options & OPT_CORRECTION) {
		for (uint8_t l = 0; l < RGB_LED_COUNT; l++, i+=6) {
			led_info_t info = config.led_infos[l];

			uint16_t x = (args[i+0] << 8) + args[i+1];
			uint16_t y = (args[i+2] << 8) + args[i+3];
			uint16_t Y = (args[i+4] << 8) + args[i+5];

			uint16_t rgb[3];
			color_correct(info, x, y, Y, rgb);

			for (int c = 0; c < 3; c++) {
				error_t error = pwm_set_brightness(info.channels[c], rgb[c]);
				if (error) return error;
			}
		}
	} else {
		for (uint8_t c = 0; c < MODULE_LENGTH; c++, i+=2) {
			uint16_t value = (args[i] << 8) + args[i+1];
			uint8_t pwm_channel = convert_channel_index(c);

			error = pwm_set_brightness(pwm_channel, value);

			if (error) return error;
		}
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
		return run_set_leds(command + 1);
	} else if (command[0] == CMD_STROBE) {
		return pwm_send_frame();
	}

	// else
	return E_WRONGCOMMAND;
}
