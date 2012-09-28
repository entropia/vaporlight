#ifndef CONSOLE_H
#define CONSOLE_H

/*
 * This module manages the debug and configuration USART (USART1). It
 * includes the function that asks for the desired operating mode
 * (debug or config) and a configuration shell.
 */

#include "error.h"
#include "command.h"

typedef enum {
	PROMPT,   // Normal mode for entering commands
	WP_ADJUST // Interactive whitepoint adjustment mode.
} console_operation_t;

/*
 * The maximum base in which parse_int can operate.
 */
#define MAX_BASE 36

/*
 * Asks the possible user if config mode should be selected.
 *
 * This is done by printing a message and checking if a key was
 * pressed after a certain timeout.
 */
vl_mode_t console_ask_mode();

/*
 * Sets the console operation mode. This affects the way in which the
 * console's status is shown and how commands are interpreted.
 */
void console_set_operation(console_operation_t op);

/*
 * Runs the configuration console. This function may or may not
 * return, depending on whether the user chose to continue running or
 * reset the board.
 */
void console_run();

/*
 * Parses the integer beginning at position *pos in line. After
 * parsing, *pos is updated to point to the first character after the
 * integer and the parsed integer is stored in *target.
 *
 * If no valid integer was found, E_ARG_FORMAT is returned, and *pos and *target are
 * unchanged.
 */
error_t parse_int(char *line, int *pos, unsigned int *target, int base);

#endif
