/*
 * This module provides a console for configuring the LED-board using
 * the debug USART. Before this module is used, debug_init must be
 * called.
 */

#include "console.h"

#include <ctype.h>

#include "config.h"
#include "console_prompt.h"
#include "console_wp.h"
#include "debug.h"
#include "error.h"
#include "term.h"

#include "stm_include/stm32/usart.h"

#define LINE_LENGTH 80


static const char *ASK_MODE_MESSAGE =
	TERM_CLEAR "Press almost any key to enter config mode..." CRLF;

static const char *ENTER_CONFIG_MODE =
	"Entering config mode." CRLF;

static const char *ENTER_NORMAL_MODE =
	"Entering normal mode." CRLF;

/*
 * Asks the possible user if config mode should be selected.
 *
 * This is done by printing a message and checking if a key was
 * pressed after a certain timeout.
 */
vl_mode_t console_ask_mode() {
	debug_string(ASK_MODE_MESSAGE);

	// Clear input
	USART1_SR &= ~USART_SR_RXNE;

	// We don't have interrupts yet, so we must busy wait a while
	// for the answer to come in (or not) One loop takes roughly 4
	// cycles, so this should wait ASK_MODE_TIMEOUT milliseconds.
#ifdef SHORT_LOOPS
	for (int j = 0; j < 3; j++) {
#else
	for (int j = 0; j < ASK_MODE_TIMEOUT * 6000; j++) {
#endif
		__asm("nop");
	}

	// Look to see if there is an answer.
	if (USART1_SR & USART_SR_RXNE) {
		volatile char dump = USART1_DR;
		(void) dump;
		debug_string(ENTER_CONFIG_MODE);
		return CONFIG_MODE;
	} else {
		debug_string(ENTER_NORMAL_MODE);
		return NORMAL_MODE;
	}

}

/*
 * The function responsible for printing the current status onto the
 * screen.
 */
static void (*show_status)() = show_status_prompt;

/*
 * The function responsible for interpreting and running commands. It
 * should return 1, if the console is to be exited.
 */
static int (*run_console_command)() = run_command_prompt;

/*
 * Sets the console operation mode. This affects the way in which the
 * console's status is shown and how commands are interpreted.
 */
void console_set_operation(console_operation_t op) {
	switch (op) {
	case PROMPT:
		show_status = show_status_prompt;
		run_console_command = run_command_prompt;
		break;
	case WP_ADJUST:
		show_status = show_status_wp;
		run_console_command = run_command_wp;
		break;
	default:
#ifdef SIMULATION
		debug_string("BUG: Unknown console operation");
#else
		error(ER_BUG, STR_WITH_LEN("Unknown console operation"), EA_PANIC);
#endif
		break;
	}
}


static const char *CONFIG_IS_INVALID =
	"The current state of configuration is invalid." CRLF;

/*
 * Runs the configuration console. This function may or may not
 * return, depending on whether the user chose to continue running or
 * reset the board.
 */
void console_run() {
	int exit;
	while(1) {
		show_status();
		exit = run_console_command();

		if (exit) {
			if (config_valid(config)) {
				break;
			} else {
				debug_string(CONFIG_IS_INVALID);
			}
		}
	}
}

/*
 * The following functions are not used by the console itself, but are
 * shared between the operating modes.
 */

/*
 * Returns the value of the given char when interpreted as a digit
 * where '0' = 0, ..., '9' = 9, 'a' = 'A' = 10, ..., 'z' = 'Z' = 35.
 * If the character cannot be interpreted as a digit, returns MAX_BASE.
 */
static int digit_value(char digit) {
	if (isdigit((int)digit)) {
		return digit - '0';
	} else if (isalpha((int)digit)){
		return tolower((int)digit) - 'a' + 10;
	} else {
		return MAX_BASE;
	}
}

/*
 * Parses the integer beginning at position *pos in line. After
 * parsing, *pos is updated to point to the first character after the
 * integer and the parsed integer is stored in *target.
 *
 * If no valid integer was found, E_ARG_FORMAT is returned, and *pos and *target are
 * unchanged.
 */
error_t parse_int(char *line, int *pos, unsigned int *target, int base) {
	int result = 0;
	int p;

	if (base > MAX_BASE) {
#ifdef SIMULATION
		debug_string("parse_int: base out of range");
		debug_int(base, 5);
		exit(-1);
#else
		error(ER_BUG, STR_WITH_LEN("parse_int: base out of range"), EA_PANIC);
#endif
	}

	for (p = *pos; digit_value(line[p]) < base; p++) {
		int digit = digit_value(line[p]);
		result = (result * base) + digit;
	}

	// We have now read all digits.
	// A space or the end of the line should follow.
	if (isspace((int)(line[p])) || line[p] == '\0') {
		*target = result;
		*pos = p;
		
		return E_SUCCESS;
	} else {
		return E_ARG_FORMAT;
	}
}

