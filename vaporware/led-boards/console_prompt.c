#include "console_prompt.h"

#include <ctype.h>
#include <stdio.h>

#include "config.h"
#include "console.h"
#include "error.h"
#include "gamma.h"
#include "git_version.h"
#include "pwm.h"
#include "term.h"

#include "stm_include/stm32/scb.h"

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

static const char *CONFIG_IS_INVALID =
	"The current state of configuration is invalid." CRLF;

static const char *GAMMA_VALUE_OUT_OF_RANGE =
	"The gamma value is out of range (0 to 0xffff)." CRLF;

static const char *RELOADING_CONFIG =
	"Reloading configuration..." CRLF;

static const char *SAVING_CONFIG =
	"Saving configuration..." CRLF;

static const char *RELOADING_GAMMA =
	"Reloading gamma table..." CRLF;

static const char *SAVING_GAMMA =
	"Saving gamma table..." CRLF;

static const char *BEGINNING_ECHO =
	"Echoing... Finish with q on a single line" CRLF;

static const char *PASTE_NOW =
	"Paste a file with one command per line, finish with q" CRLF;

/*
 * Checks that the given value is greater than or equal to 0 and less
 * then the given limit. Prints the given message and returns
 * E_ARG_FORMAT if this is not the case. Returns E_SUCCESS otherwise.
 */
static error_t check_range(int value, int limit, const char *message) {
	if (value < 0 || value >= limit) {
		console_write(message);
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
		console_write(ADDR_OUT_OF_RANGE);
		return E_ARG_FORMAT;
	}
	if (addr == 0xfd) {
		console_write(WARN_BROADCAST_ADDR);
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
 * Also passes on the errors form pwm_set_brightness.
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

	error_t error = pwm_set_brightness(config.physical_led[index], (uint8_t) brightness);
	if (error) return error;
	return pwm_send_frame();
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
		console_write(COLOR_CODE_INVALID);
		return E_ARG_FORMAT;
	}

	config.led_color[config.physical_led[index]] = color;
	return E_SUCCESS;
}

/*
 * Runs the "enter echo mode" command.
 *
 * Expected format for args: { }
 *
 * Always succeeds.
 */
static error_t run_echo(unsigned int args[]) {
	char buf[80];

	console_write(BEGINNING_ECHO);

	do {
		console_getline(buf, 80);
	} while (strncmp(buf, "q", 80));

	return E_SUCCESS;
}

/*
 * Runs the "paste command file" command.
 *
 * Expected format for args: { }
 *
 * Always succeeds (although the commands in the file may not).
 */
static error_t run_paste_file(unsigned int args[]) {
	console_write(PASTE_NOW);

	int should_exit = 0;

	do {
		should_exit = run_command_prompt();
	} while(!should_exit);

	return E_SUCCESS;
}

/*
 * Runs the "set gamma table" command.
 *
 * Expected format for args: { color-index, raw-brightness, gamma-value }
 *
 * Returns E_ARG_FORMAT if either index is out of range.
 */
static error_t run_set_gamma(unsigned int args[]) {
	int color = args[0];
	int raw = args[1];
	int gamma = args[2];

	if (color != RED && color != GREEN && color != BLUE && color != WHITE) {
		console_write(COLOR_CODE_INVALID);
		return E_ARG_FORMAT;
	}
	if (check_byte(raw, BRIGHTNESS_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}
	if (check_short(gamma, GAMMA_VALUE_OUT_OF_RANGE)) {
		return E_ARG_FORMAT;
	}

	gamma_edit(color, raw, gamma);
	return E_SUCCESS;
}

/*
 * Runs the "dump gamma table" command.
 *
 * Expected format for args: { }
 *
 * Always succeeds.
 */
static error_t run_dump_gamma(unsigned int args[]) {

	for (unsigned b = 0; b <= 0xff; b++) {
		console_int_02x(b);
		console_write(": ");

		// TODO has cardinality of color_t harcoded!
		for (uint8_t c = 0; c < 4; c++) {
			console_int_04x(gamma(c, (uint8_t) b));
			console_write(" ");
		}

		console_write(CRLF);
	}

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

	console_write(RELOADING_CONFIG);

	error_t error = load_config();

	switch (error) {
	case E_SUCCESS:
		break;
	case E_NOCONFIG:
		console_write(NO_CONFIG_FOUND);
		break;
	default:
		console_write(UNKNOWN_FLASH_ERROR);
		break;
	}

	console_write(RELOADING_GAMMA);

	error = gamma_reload();

	if (error != E_SUCCESS) {
		console_write(UNKNOWN_FLASH_ERROR);
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
	SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;

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
	if (!config_valid(config)) {
		console_write(CONFIG_IS_INVALID);
		return E_NOCONFIG;
	}

	console_write(SAVING_CONFIG);

	error_t error = save_config();

	switch(error) {
	case E_SUCCESS:
		break;
	case E_FLASH_WRITE:
		console_write(FLASH_WRITE_FAILED);
		break;
	default:
		console_write(UNKNOWN_FLASH_ERROR);
		break;
	}

	console_write(SAVING_GAMMA);

	error = gamma_save();

	switch(error) {
	case E_SUCCESS:
		break;
	case E_FLASH_WRITE:
		console_write(FLASH_WRITE_FAILED);
		break;
	default:
		console_write(UNKNOWN_FLASH_ERROR);
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
 * This is actually implemented after the command array, because it
 * needs access to it.
 */
static error_t run_help(unsigned int args[]);

static console_command_t commands[] = {
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
		.key = 'e',
		.arg_length = 0,
		.handler = run_echo,
		.usage = "e: Begin echo mode",
		.does_exit = 0,
	},
	{
		.key = 'f',
		.arg_length = 0,
		.handler = run_paste_file,
		.usage = "f: Paste a command file",
		.does_exit = 0,
	},
	{
		.key = 'g',
		.arg_length = 3,
		.handler = run_set_gamma,
		.usage = "g <color-index> <raw-brightness> <gamma-value>: Set gamma table",
		.does_exit = 0,
	},
	{
		.key = 'G',
		.arg_length = 0,
		.handler = run_dump_gamma,
		.usage = "G: Dump gamma table",
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
		.usage = "l: Reload configuration and gamma table from flash",
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
		.usage = "s: Save configuration and gamma table to flash",
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
#define COMMAND_COUNT (sizeof(commands) / sizeof(console_command_t))
#define MAX_ARG_LEN 3

/*
 * Runs the "show help" command.
 *
 * Expected format for args: { }
 *
 * This function always succeeds and returns E_SUCCESS;
 */
static error_t run_help(unsigned int args[]) {
	for (int i = 0; i < COMMAND_COUNT; i++) {
		console_write(commands[i].usage);
		console_write(CRLF);
	}

	console_write(CRLF);

	return E_SUCCESS;
}

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


	console_write(PROGRAM_ID);

	console_write(MODULE_ADDRESS); console_int_02x(config.my_address); console_write(CRLF CRLF);

	console_write(LED_SETTINGS_HEAD);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		int phy = config.physical_led[i];

		console_write("  ");
		console_int_01x(i);
		console_write("    ");
		console_int_01x(phy);
		console_write("  ");
		console_int_04x(config.white_correction[phy]);
		console_write("      ");
		console_int_01x(config.led_color[phy]);
		console_write(CRLF);
	}
	console_write(CRLF);

	console_write(HEAT_SETTINGS_HEAD);

	for (int i = 0; i < HEAT_SENSOR_LEN; i++) {
		console_write("     ");
		console_int_01x(i);
		console_write("   ");
		console_int_04x(config.heat_limit[i]);
		console_write(CRLF);
	}
	console_write(CRLF);

	console_write(CONSOLE_PROMPT);
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

	console_getline(line, LINE_LENGTH);

	console_command_t *comm = get_command(line[0]);
	if (comm) {
		error_t error = parse_args(line, args, comm->arg_length);
		if (error != E_SUCCESS) {

			if (error == E_MISSING_ARGS) {
				console_write(ARGUMENTS_ARE_MISSING);
			} else if (error == E_ARG_FORMAT) {
				console_write(ARGUMENTS_ARE_INVALID);
			} else {
				console_write(UNKNOWN_PARSER_ERROR);
			}

			console_write(USAGE);
			console_write(comm->usage);
			console_write(CRLF);

			return 0;
		}

		if (comm->handler(args) != E_SUCCESS) {
			console_write(ERROR_RUNNING_COMMAND);
			return 0;
		}

		return comm->does_exit;
	} else {
		console_write(WRONG_COMMAND);
		return 0;
	}
}
