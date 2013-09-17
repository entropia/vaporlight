#ifndef CONSOLE_H
#define CONSOLE_H

/*
 * This module manages the debug and configuration USART (USART1). It
 * includes the function that asks for the desired operating mode
 * (debug or config) and a configuration shell.
 */

#include "error.h"
#include "fixedpoint.h"
#include "command.h"

/*
 * The maximum base in which parse_int can operate.
 */
#define MAX_BASE 36

/*
 * Low-level functions wrapping usart1.
 */

/*
 * Prints one character on the console.
 */
void console_putchar(const char message);

/*
 * Prints raw data on the console.
 */
void console_write_raw(const char *message, unsigned length);

/*
 * Prints a string on the console.
 */
#define console_write(str) console_write_raw(STR_WITH_LEN(str))

/*
 * Prints an integer to the console, formatting it in the given base
 * and padding it to the given minimum width with the given padding
 * character. Abbreviating macros are available, following the format
 * conventions of printf.
 */
void console_uint(unsigned value, unsigned base, int min_width, char padding);

#define console_uint_d(value)   console_uint(value, 10, 0, ' ')
#define console_uint_0d(value)  console_uint(value, 10, 0, '0')
#define console_uint_2d(value)  console_uint(value, 10, 2, ' ')
#define console_uint_3d(value)  console_uint(value, 10, 3, ' ')
#define console_uint_4d(value)  console_uint(value, 10, 4, ' ')
#define console_uint_5d(value)  console_uint(value, 10, 5, ' ')
#define console_uint_x(value)   console_uint(value, 16, 0, ' ')
#define console_uint_0x(value)  console_uint(value, 16, 0, '0')
#define console_uint_01x(value) console_uint(value, 16, 1, '0')
#define console_uint_02x(value) console_uint(value, 16, 2, '0')
#define console_uint_04x(value) console_uint(value, 16, 4, '0')
#define console_uint_08x(value) console_uint(value, 16, 8, '0')

void console_sint(int value, unsigned base, int min_width, char padding);

#define console_sint_d(value)   console_sint(value, 10, 0, ' ')
#define console_sint_0d(value)  console_sint(value, 10, 0, '0')
#define console_sint_2d(value)  console_sint(value, 10, 2, ' ')
#define console_sint_3d(value)  console_sint(value, 10, 3, ' ')
#define console_sint_4d(value)  console_sint(value, 10, 4, ' ')
#define console_sint_5d(value)  console_sint(value, 10, 5, ' ')
#define console_sint_x(value)   console_sint(value, 16, 0, ' ')
#define console_sint_0x(value)  console_sint(value, 16, 0, '0')
#define console_sint_01x(value) console_sint(value, 16, 1, '0')
#define console_sint_02x(value) console_sint(value, 16, 2, '0')
#define console_sint_04x(value) console_sint(value, 16, 4, '0')
#define console_sint_08x(value) console_sint(value, 16, 8, '0')

/*
 * Prints a fixed-point number to the console, formatting it
 * in the given base.
 */
void console_fixed(fixed_t value, unsigned base);

/*
 * Returns the next character received on the console.
 * This function blocks until a character is available.
 *
 * If an error condition is signalled *during the wait*,
 * the function returns 0xff.
 */
char console_getchar();

/*
 * Reads a complete line into the given buffer.  The line may be at
 * most count characters long. Any further input except for "Return"
 * to finish the line is discarded and not echoed.
 */
void console_getline(char *buffer, int count);

/*
 * Functions for running the configuration console.
 */

/*
 * Asks the possible user if config mode should be selected.
 *
 * This is done by printing a message and checking if a key was
 * pressed after a certain timeout.
 */
vl_mode_t console_ask_mode();

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

/*
 * Prompts for an integer input until a valid integer is entered.
 */
unsigned int console_ask_int(const char *prompt, int base);

#endif
