#include "command.h"

#include "color.h"
#include "config.h"
#include "console.h"
#include "debug.h"
#include "pwm.h"
#include "usart2.h"
#include "term.h"

/*
 * The broadcast address.
 */
static const uint8_t BROADCAST = 0xff;

typedef enum {
	CMD_SET_RAW = 0x00,
	CMD_SET_XYY = 0x01,
	CMD_STROBE = 0xff
} commant_t;

/*
 * The USART address filter function.
 *
 * This function accepts the address of the module, the broadcast
 * address and all special commands.
 */
static bool address_filter(uint8_t address) {
#ifdef TRACE_USART
	debug_string("t");
	debug_putchar(config.my_address);
#endif

	return address == config.my_address ||
		address == BROADCAST;
}

/*
 * The USART length check function.
 *
 * This function checks the first byte of command_prefix (which is the
 * command code) and calculates the remaining bytes necessary from the
 * fixed length of each command.
 */
static int length_check(uint8_t *command_prefix, int length_so_far) {
	if (length_so_far == 0) {
		// This should not even happen, but better be prepared.
		// We want to see at least the command code.
		return 1;
	} else {
		uint8_t cmd_code = command_prefix[0];

		int total_length;
		switch(cmd_code) {
		case CMD_SET_RAW:
			total_length = 1 + (sizeof(uint16_t) * MODULE_LENGTH);
			break;
		case CMD_SET_XYY:
			total_length = 1 + (sizeof(uint16_t) * 3 * RGB_LED_COUNT);
			break;
		case CMD_STROBE:
			total_length = 1;
			break;
		default:
			// Abort reception as early as
			// possible. However, the invalid command will
			// still be given to run_command eventually.
			total_length = 0;
		}

		return total_length - length_so_far;
	}
}

/*
 * Initializes the command module and sets up the USART filter
 * appropriately.
 */
void command_init() {
	usart2_set_address_filter(address_filter);
	usart2_set_length_check(length_check);
}

/*
 * Runs a "set LEDs raw" command.
 */
static error_t run_set_raw(uint8_t *args) {
#ifdef TRACE_COMMANDS
	console_write("raw");
#endif
	int i = 0;

	for (uint8_t c = 0; c < MODULE_LENGTH; c++, i+=2) {
		uint16_t value = (args[i] << 8) + args[i+1];
		uint8_t pwm_channel = convert_channel_index(c);

		error_t error = pwm_set_brightness(pwm_channel, value);

		if (error) return error;
	}

	return E_SUCCESS;
}

/*
 * Runs a "set LEDs xyY" command.
 */
static error_t run_set_xyY(uint8_t *args) {
#ifdef TRACE_COMMANDS
	console_write("xyY");
#endif
#ifdef COUNT_SET_LEDS
	cycle_start();
#endif

	int i = 0;
	for (uint8_t l = 0; l < RGB_LED_COUNT; l++, i+=6) {
		led_info_t info = config.led_infos[l];

		uint16_t x = (args[i+0] << 8) + args[i+1];
		uint16_t y = (args[i+2] << 8) + args[i+3];
		uint16_t Y = (args[i+4] << 8) + args[i+5];

		uint16_t rgb[3];
		color_correct(info, x, y, Y, rgb);

#ifdef TRACE_COMMANDS
		console_uint_d(l); console_write(" ");
		console_uint_d(x); console_write(" ");
		console_uint_d(y); console_write(" ");
		console_uint_d(Y); console_write(" ");
		console_uint_d(rgb[0]); console_write(" ");
		console_uint_d(rgb[1]); console_write(" ");
		console_uint_d(rgb[2]); console_write(" ");
#endif

		for (int c = 0; c < 3; c++) {
			error_t error = pwm_set_brightness(info.channels[c], rgb[c]);
			if (error) return error;
		}
	}

#ifdef COUNT_SET_LEDS
	int cycles = cycle_get();
	console_write("Set LED cycles: ");
	console_uint_d(cycles);
	console_write(CRLF);
#endif
#ifdef TRACE_COMMANDS
	console_write("Done" CRLF);
#endif
	return E_SUCCESS;
}

/*
 * Runs the command pointed to by 'command'.
 *
 * Returns an error/success code.
 */
error_t run_command(uint8_t *command) {
	switch(command[0]) {
	case CMD_SET_RAW:
		return run_set_raw(command + 1);
		break;
	case CMD_SET_XYY:
		return run_set_xyY(command + 1);
		break;
	case CMD_STROBE:
#ifdef TRACE_COMMANDS
		console_write("!");
#endif
		return pwm_send_frame();
		break;
	default:
		return E_WRONGCOMMAND;
	}
}
