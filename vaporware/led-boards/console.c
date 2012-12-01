/*
 * This module provides a console for configuring the LED-board using
 * usart1. Before this module is used, usart1_init must be called.
 */

#include "console.h"

#include <ctype.h>

#include "config.h"
#include "console_prompt.h"
#include "console_wp.h"
#include "error.h"
#include "term.h"
#include "usart1.h"

#include "stm_include/stm32/usart.h"

#define LINE_LENGTH 80


static const char *ASK_MODE_MESSAGE =
	TERM_CLEAR "Press almost any key to enter config mode..." CRLF;

static const char *ENTER_CONFIG_MODE =
	"Entering config mode." CRLF;

static const char *ENTER_NORMAL_MODE =
	"Entering normal mode." CRLF;

/*
 * Low-level functions wrapping usart1.
 */

/*
 * Prints one character on the console.
 */
void console_putchar(const char message) {
	usart1_putchar(message);
}

/*
 * Prints raw data on the console.
 */
void console_write_raw(const char *message, int length) {
	for (int i = 0; i < length; i++) {
		usart1_putchar(message[i]);
	}
}

// console_write is a macro defined in console.h

/*
 * Prints an integer to the console, formatting it in the given base
 * and padding it to the given minimum width with the given padding
 * character. Abbreviating macros are available, following the format
 * conventions of printf.
 */
void console_int(int value, int base, int min_width, char padding) {
#define INT_WIDTH 32

	char buf[INT_WIDTH]; // Enough for 32 bit integers in binary.
	int pos = INT_WIDTH;

	do {
		int digit = value % base;

		if (0 <= digit && digit <= 9) {
			buf[--pos] = '0' + digit;
		} else {
			buf[--pos] = 'a' + (digit - 10);
		}
		
		value /= base;
	} while (value > 0);

	int number_len = INT_WIDTH - pos;
	for (int i = 0; i < min_width - number_len; i++) {
		console_putchar(padding);
	}
	for (int i = pos; i < INT_WIDTH; i++) {
		console_putchar(buf[i]);
	}
}

/*
 * Returns the next character received on the console.
 * This function blocks until a character is available.
 *
 * If an error condition is signalled *during the wait*,
 * the function returns 0xff.
 */
char console_getchar() {
	return usart1_getchar();
}

/*
 * Reads a complete line into the given buffer.  The line may be at
 * most count characters long. Any further input except for "Return"
 * to finish the line is discarded and not echoed.
 */
void console_getline(char *buffer, int count) {
	int i = 0; // Index where the next char is to be stored.
	char input;

	for (i = 0; i < count; i++) {
		buffer[i] = '\0';
	}

	i = 0;

	while(1) {
		input = console_getchar();

		switch(input) {
		case '\n':
		case '\r':
			// Return is used to terminate the input.
			console_write("\r\n");
			return;
		case '\b':
			if (i > 0) {
				i--;
				buffer[i] = '\0';
				console_write("\b \b");
			}
			break;
		case '\033': // Escape
			// We do not handle escape sequences here
			// (such as arrow keys etc). This will leave
			// garbage on the screen but at least not
			// break anything. In order to actually ignore
			// a complete escape sequence, we would have
			// to implement nearly a full parser here.
			continue;
		default:
			if (i < count) {
				buffer[i] = input;
				console_putchar(input);
				i++;
			} else {
				console_write("X\b");
			}
			break;
		}
	}
}

/*
 * Functions for running the configuration console.
 */

/*
 * Asks the user if config mode should be selected.
 *
 * This is done by printing a message and checking if a key was
 * pressed after a certain timeout.
 */
vl_mode_t console_ask_mode() {
	console_write(ASK_MODE_MESSAGE);

	// Clear input
	USART1_SR &= ~USART_SR_RXNE;

	// One loop takes roughly 4 cycles, so this should wait
	// ASK_MODE_TIMEOUT milliseconds.
#ifdef SHORT_LOOPS
	for (int j = 0; j < 3; j++) {
#else
	for (int j = 0; j < ASK_MODE_TIMEOUT * 6000; j++) {
#endif
		__asm("nop");
	}

	// Look to see if there is an answer.
	
	// Since the user may actually have pressed several keys and
	// these should not appear in the console, we must read the
	// whole input buffer.
	if (usart1_has_input()) {
		while(usart1_has_input()) {
			usart1_getchar();
		}

		console_write(ENTER_CONFIG_MODE);
		return CONFIG_MODE;
	} else {
		console_write(ENTER_NORMAL_MODE);
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
		error(ER_BUG, STR_WITH_LEN("Unknown console operation"), EA_PANIC);
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
				console_write(CONFIG_IS_INVALID);
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
		error(ER_BUG, STR_WITH_LEN("parse_int: base out of range"), EA_PANIC);
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

