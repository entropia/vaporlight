#include "console_prompt.h"

#include <ctype.h>
#include <stdio.h>

#include "config.h"
#include "console.h"
#include "debug.h"
#include "error.h"
#include "git_version.h"
#include "led.h"
#include "term.h"

#ifndef SIMULATION
#include "stm_include/stm32/scb.h"
#endif

/*
 * Expanded Stringify and Stringify macros.
 * Taken from http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 */
#define XSTR(s) STR(s)
#define STR(s) #s

/*
 * Function type for config console commands.  The function receives
 * the parameters given on the console already parsed as integers.
 */
typedef error_t (*command_handler_t)(unsigned int[]);

/*
 * This represents one command possible on the console.
 */
typedef struct {
	char key; // The key with which the command is invoked.
	int arg_length; // The number of arguments expected.
	command_handler_t handler; // The handler function for this command.
	char *usage; // A message to display when the command has not
		     // been successful or as a help text.
	int does_exit; // The config console should exit after this
		       // command has been successfully run.
} console_command_t;

/*
 * Forward declaration for commands which need to read the command table.
 */
static console_command_t commands[];

#define COMMAND_COUNT 12
#define MAX_ARG_LEN 3
#define LINE_LENGTH 80

static const char *ADDR_OUT_OF_RANGE =
	"The address is out of range (0x00 to 0xfd)" CRLF;

static const char *WARN_BROADCAST_ADDR =
	"Warning: Setting address to the broadcast address" CRLF;

static const char *LED_OUT_OF_RANGE =
	"The LED index is out of range (0 to " XSTR(MODULE_LENGTH) "-1)" CRLF;

static const char *BRIGHTNESS_OUT_OF_RANGE =
	"The brightness is out of range (0 to 0xff)" CRLF;

static const char *COLOR_CODE_INVALID =
	"The color code is invalid (0, 1, 2, 3)" CRLF;

static const char *SENSOR_OUT_OF_RANGE =
	"The heat sensor index is out of range (0 to "
	XSTR(HEAT_SENSOR_LEN) "-1)" CRLF;

static const char *HEAT_LIMIT_OUT_OF_RANGE =
	"The heat limit is out of range (0 to 0xffff)" CRLF;

static const char *NO_CONFIG_FOUND =
	"No configuration has been found in flash" CRLF;

static const char *UNKNOWN_FLASH_ERROR =
	"Unknown internal flash error" CRLF;

static const char *LOGICAL_OUT_OF_RANGE =
	"The logical LED index is out of range (0 to " XSTR(MODULE_LENGTH) "-1)" CRLF;

static const char *PHYSICAL_OUT_OF_RANGE =
	"The phyiscal LED index is out of range (0 to " XSTR(MODULE_LENGTH) "-1)" CRLF;

static const char *FLASH_WRITE_FAILED =
	"Writing to flash failed. Maybe this board is getting old." CRLF;

static const char *CORRECTION_OUT_OF_RANGE =
	"The whitepoint correction is out of range (0 to 0xffff)" CRLF;

/*
 * Checks that the given value is greater than or equal to 0 and less
 * then the given limit. Prints the given message and returns
 * E_ARG_FORMAT if this is not the case. Returns E_SUCCESS otherwise.
 */
static error_t check_range(int value, int limit, const char *message) {
	if (value < 0 || value >= limit) {
		debug_string(message);
		return E_ARG_FORMAT;
	} else {
		return E_SUCCESS;
	}
}

/*
 * Checks that the given index is a valid LED index.
 */
static error_t check_led_index(int index, const char *message) {
	return check_range(index, MODULE_LENGTH, message);
}

/*
 * Checks that the given index is an unsigned 8-bit value.
 */
static error_t check_byte(int value, const char *message) {
	return check_range(value, 0x100, message);
}

/*
 * Checks that the given index is an unsigned 16-bit value.
 */
static error_t check_short(int value, const char *message) {
	return check_range(value, 0x10000, message);
}

/*
 * Runs the "set module address" command.
 *
 * Expected format for args: { address }
 *
 * Returns E_ARG_FORMAT if the given address is out of range.
 */
static error_t run_set_addr(unsigned int args[]) {
	int addr = args[0];

	// The allowable range for an address is 0x00 to 0xfe
	// (0xff is reserved)
	if (addr < 0 || addr > 0xfd) {
		debug_string(ADDR_OUT_OF_RANGE);
		return E_ARG_FORMAT;
	}
	if (addr == 0xfd) {
		debug_string(WARN_BROADCAST_ADDR);
	}

	config.my_address = addr;
	return E_SUCCESS;
}

/*
 * Runs the "set brightness" command.
 *
 * Expected format for args: { led-index, brightness }
 *
 * Returns E_ARG_FORMAT if the LED index or brightness is out of range.
 * Also passes on the errors form led_set_brightness.
 */
static error_t run_set_brightness(unsigned int args[]) {
	int index = args[0];
	int brightness = args[1];

	if (check_led_index(index, LED_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (check_byte(brightness, BRIGHTNESS_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}

#ifdef SIMULATION
	debug_string("Imagine a LED glowing...");
	return E_SUCCESS;
#else
	error_t error = led_set_brightness(config.physical_led[index], (uint8_t) brightness);
	if (error) return error;
	return led_send_frame();
#endif
}

/*
 * Runs the "set LED color" command.
 *
 * Expected format for args: { led-index, color }
 *
 * Returns E_ARG_FORMAT if the LED index is out of range or the
 * color code is invalid.
 */
static error_t run_set_color(unsigned int args[]) {
	int index = args[0];
	int color = args[1];

	if (check_led_index(index, LED_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (color != RED && color != GREEN && color != BLUE && color != WHITE) {
		debug_string(COLOR_CODE_INVALID);
		return E_ARG_FORMAT;
	}

	config.led_color[config.physical_led[index]] = color;
	return E_SUCCESS;
}

/*
 * Runs the "set heat limit" command.
 *
 * Expected format for args: { sensor-index, heat-limit }
 *
 * Returns E_ARG_FORMAT if the heat sensor index or the limit is out of range.
 */
static error_t run_set_heat_limit(unsigned int args[]) {
	int index = args[0];
	int limit = args[1];

	if (check_range(index, HEAT_SENSOR_LEN, SENSOR_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (check_short(limit, HEAT_LIMIT_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}

	config.heat_limit[index] = limit;
	return E_SUCCESS;
}

/*
 * Runs the "reload configuration from flash" command.
 *
 * Expected format for args: {  }
 *
 * Returns the error reported by load_config.
 */
static error_t run_reload_config(unsigned int args[]) {
	error_t error = load_config();

	switch (error) {
	case E_SUCCESS:
		break;
	case E_NOCONFIG:
		debug_string(NO_CONFIG_FOUND);
		break;
	default:
		debug_string(UNKNOWN_FLASH_ERROR);
		break;
	}

	return error;
}

/*
 * Runs the "set LED permutation" command.
 *
 * Expected format for args: { logical-index, physical-index }
 *
 * Returns E_ARG_FORMAT if any of the indices are out of range.
 */
static error_t run_set_permutation(unsigned int args[]) {
	int logical = args[0];
	int physical = args[1];

	if (check_led_index(logical, LOGICAL_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (check_led_index(physical, PHYSICAL_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}

	config.physical_led[logical] = physical;
	return E_SUCCESS;
}

/*
 * Runs the "quit" command.
 *
 * This function always succeeds and returns E_SUCCESS.
 */
static error_t run_quit(unsigned int args[]) {
	return E_SUCCESS;
}

/*
 * Runs the "reset module" command.
 *
 * Expected format for args: { }
 *
 * This function does not return.
 */
static error_t run_reset(unsigned int args[]) {
#ifdef SIMULATION
	debug_string("Simulation only. I will not reset your PC" CRLF);
#else
	SCB_AIRCR |= SCB_AIRCR_SYSRESETREQ;
#endif

	return E_SUCCESS;
}

/*
 * Runs the "save configuration to flash" command.
 *
 * Expected format for args: { }
 *
 * Returns the error reported by save_config.
 */
static error_t run_save_config(unsigned int args[]) {
	error_t error = save_config();

	switch(error) {
	case E_SUCCESS:
		break;
	case E_FLASH_WRITE:
		debug_string(FLASH_WRITE_FAILED);
		break;
	default:
		debug_string(UNKNOWN_FLASH_ERROR);
		break;
	}

	return error;
}

/*
 * Runs the "set whitepoint correction" command.
 *
 * Expected format for args: { led-index, correction }
 *
 * Returns E_ARG_FORMAT if the LED index ro whitepoint correction are
 * out of range.
 */
static error_t run_set_whitepoint(unsigned int args[]) {
	int index = args[0];
	int correction = args[1];

	if (check_led_index(index, LED_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (check_short(correction, CORRECTION_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}

	config.white_correction[config.physical_led[index]] = correction;
	return E_SUCCESS;
}

/*
 * Runs the "switch to whitepoint adjustment operation" command.
 *
 * Expected format for args: { }
 *
 * This function always succeeds and returns E_SUCCESS;
 */
static error_t run_switch_to_wp(unsigned int args[]) {
	console_set_operation(WP_ADJUST);
	
	return E_SUCCESS;
}

/*
 * Runs the "show help" command.
 *
 * Expected format for args: { }
 *
 * This function always succeeds and returns E_SUCCESS;
 */
static error_t run_help(unsigned int args[]) {
	for (int i = 0; i < COMMAND_COUNT; i++) {
		debug_string(commands[i].usage);
		debug_string(CRLF);
	}

	debug_string(CRLF);
	
	return E_SUCCESS;
}

static console_command_t commands[COMMAND_COUNT] = {
	{
		.key = 'a',
		.arg_length = 1,
		.handler = run_set_addr,
		.usage = "a <address>: Set module address",
		.does_exit = 0,
	},
	{
		.key = 'b',
		.arg_length = 2,
		.handler = run_set_brightness,
		.usage =
		"b <led-index> <brightness>: Set brightness of a single LED",
		.does_exit = 0,
	},
	{
		.key = 'c',
		.arg_length = 2,
		.handler = run_set_color,
		.usage = "c <led-index> <color>: Set color",
		.does_exit = 0,
	},
	{
		.key = 'h',
		.arg_length = 2,
		.handler = run_set_heat_limit,
		.usage = "h <sensor-index> <heat-limit>: Set heat limit",
		.does_exit = 0,
	},
	{
		.key = 'l',
		.arg_length = 0,
		.handler = run_reload_config,
		.usage = "l: Reload configuration from flash",
		.does_exit = 0,
	},
	{
		.key = 'p',
		.arg_length = 2,
		.handler = run_set_permutation,
		.usage = "p <logical-index> <physical-index>: Set LED permutation",
		.does_exit = 0,
	},
	{
		.key = 'q',
		.arg_length = 0,
		.handler = run_quit,
		.usage = "q: Quit and continue in normal mode",
		.does_exit = 1,
	},
	{
		.key = 'r',
		.arg_length = 0,
		.handler = run_reset,
		.usage = "r: Reset the module",
		.does_exit = 0,
	},
	{
		.key = 's',
		.arg_length = 0,
		.handler = run_save_config,
		.usage = "s: Save configuration to flash",
		.does_exit = 0,
	},
	{
		.key = 'w',
		.arg_length = 2,
		.handler = run_set_whitepoint,
		.usage = "w <led-index> <correction>: Set whitepoint correction",
		.does_exit = 0,
	},
	{
		.key = 'W',
		.arg_length = 0,
		.handler = run_switch_to_wp,
		.usage = "W: Switch to whitepoint adjustment operation",
		.does_exit = 0,
	},
	{
		.key = '?',
		.arg_length = 0,
		.handler = run_help,
		.usage = "?: Show command usage messages",
		.does_exit = 0,
	},
};

static const char *PROGRAM_ID =
	"vaporware build " GIT_VERSION_ID CRLF;

static const char *MODULE_ADDRESS =
	"Module address: ";

static const char *LED_SETTINGS_HEAD =
	"LED settings:" CRLF
	"Log  Phy  corr  color" CRLF;

static const char *HEAT_SETTINGS_HEAD =
	"Heat sensor settings:" CRLF
        "Sensor  Limit" CRLF;

static const char *CONSOLE_PROMPT =
	"> ";

/*
 * Displays the current configuration and a prompt on the debug
 * console.
 */
void show_status_prompt() {

	// Sketch for the config console screen
/*
vaporlight build 0000000000000000000000000000000000000000
This is module 00

LED settings:
Log  Phy  corr   color
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1
  0    0  ffff      1

Heat sensor settings:
Sensor   Limit
     0   ffff
     0   ffff
     0   ffff
     0   ffff
     0   ffff
     0   ffff

>
*/


	debug_string(PROGRAM_ID);

	debug_string(MODULE_ADDRESS); debug_hex(config.my_address, 2); debug_string(CRLF CRLF);

	debug_string(LED_SETTINGS_HEAD);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		int phy = config.physical_led[i];

		debug_string("  ");
		debug_hex(i, 1);
		debug_string("    ");
		debug_hex(phy, 1);
		debug_string("  ");
		debug_hex(config.white_correction[phy], 4);
		debug_string("      ");
		debug_hex(config.led_color[phy], 1);
		debug_string(CRLF);
	}
	debug_string(CRLF);

	debug_string(HEAT_SETTINGS_HEAD);

	for (int i = 0; i < HEAT_SENSOR_LEN; i++) {
		debug_string("     ");
		debug_hex(i, 1);
		debug_string("   ");
		debug_hex(config.heat_limit[i], 4);
		debug_string(CRLF);
	}
	debug_string(CRLF);

	debug_string(CONSOLE_PROMPT);
}

/*
 * Looks for the command with the given key in the commands table
 * provided by the console_commands module and returns a pointer to
 * its console_command_t instance if found. If no command has been
 * found, returns NULL.
 */
static console_command_t *get_command(char key) {
	for (int i = 0; i < COMMAND_COUNT; i++) {
		if (commands[i].key == key) {
			return &commands[i];
		}
	}

	return NULL;
}

#define SKIP_WHILE(pred, string, position)  \
	while(pred((int)((string)[(position)]))) {	\
		(position)++;		    \
	}

/*
 * Parses a command line of the format "<command-key> <integer-argument>*".
 *
 * arg_length specifies how may arguments are expected. If there are
 * more arguments present, they are ignored; if there are less,
 * E_MISSING_ARGS is returned. The arguments are converted to integers
 * and stored in args[0]...args[arg_length-1]. If any of the
 * conversions fails, E_ARGUMENT_FORMAT is returned.
 */
static error_t parse_args(char *line, unsigned int *args, int arg_length) {
	int pos = 0;
	
	// Skip over the command-key and following space.
	SKIP_WHILE(isalnum, line, pos);
	SKIP_WHILE(isspace, line, pos);
	

	for (int arg = 0; arg < arg_length; arg++) {
		if (line[pos] == '\0') {
			// The line has ended before all args could be parsed.
			return E_MISSING_ARGS;
		}

		_Static_assert(CONSOLE_READ_BASE <= MAX_BASE, "CONSOLE_READ_BASE out of range");
		error_t error = parse_int(line, &pos, &args[arg], CONSOLE_READ_BASE);

		if (error) return error;

		SKIP_WHILE(isspace, line, pos);
	}
	
	return E_SUCCESS;
}

static const char *WRONG_COMMAND =
	"Unknown command" CRLF;

static const char *ARGUMENTS_ARE_MISSING =
	"Not enough arguments have been passed" CRLF;

static const char *ARGUMENTS_ARE_INVALID =
	"An argument was not a valid integer" CRLF;

static const char *UNKNOWN_PARSER_ERROR =
	"An error occurred while parsing your input" CRLF;

static const char *ERROR_RUNNING_COMMAND =
	"An error occured while running the command" CRLF;

static const char *USAGE =
	"Usage: ";

/*
 * Reads a command entered on the debug console and executes it
 * according to the commands array.
 *
 * Returns 1 if the console should exit and continue with normal mode.
 */
int run_command_prompt() {
	char line[LINE_LENGTH];
	unsigned int args[MAX_ARG_LEN];

	debug_getline(line, LINE_LENGTH);

	console_command_t *comm = get_command(line[0]);
	if (comm) {
		error_t error = parse_args(line, args, comm->arg_length);
		if (error != E_SUCCESS) {
			
			if (error == E_MISSING_ARGS) {
				debug_string(ARGUMENTS_ARE_MISSING);
			} else if (error == E_ARG_FORMAT) {
				debug_string(ARGUMENTS_ARE_INVALID);
			} else {
				debug_string(UNKNOWN_PARSER_ERROR);
			}

			debug_string(USAGE);
			debug_string(comm->usage);
			debug_string(CRLF);
			
			return 0;
		}
		
		if (comm->handler(args) != E_SUCCESS) {
			debug_string(ERROR_RUNNING_COMMAND);
			return 0;
		}

		return comm->does_exit;
	} else {
		debug_string(WRONG_COMMAND);
		return 0;
	}
}
