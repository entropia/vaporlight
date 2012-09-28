/*
 * This module access to the debug USART and a few other debug-related
 * functions.
 */

#include "error.h"

/*
 * Initializes the debug and configuration USART. This must be called
 * before any other function in this module (which includes the debug
 * funtions!).
 */
error_t debug_init();

/*
 * Sends a character via the "debug TX" USART.
 */
void debug_putchar(const char message);

/*
 * Sends a message via the "debug TX" USART.
 */
void debug_write(const char *message, int length);

#define debug_string(str) debug_write(STR_WITH_LEN(str))

/*
 * Converts the last "width" nybbles of x to hexadecimal format and
 * sends them via the debug USART.
 */
void debug_hex(int x, int width);

/*
 * Converts the given integer to decimal format and sends it via the
 * debug USART. If the resulting string is shorter than min_width, it
 * is padded at the left with spaces.
 */
void debug_int(unsigned int x, int min_width);

/*
 * Returns the next character received via the debug USART.
 * This function blocks until a character is available.
 *
 * If an error condition is signalled *during the wait*,
 * the function returns 0xff.
 */
char debug_getchar();

/*
 * Reads a complete line into the given buffer.  The line may be at
 * most count characters long. Any further input except for "Return"
 * to finish the line is discarded and not echoed.
 */
void debug_getline(char *buffer, int count);

/*
 * Starts the cycle counter.
 */
void cycle_start();

/*
 * Returns the number of cycles elapsed since the cycle counter was
 * started.
 */
int cycle_get();

/*
 * Stops the cycle counter and returns the number of cycles elapsed
 * since it was started.
 */
int cycle_stop();
