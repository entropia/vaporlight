/*
 * This module access to the debug USART and a few other debug-related
 * functions.
 */

#include "debug.h"

#include <stdint.h>

#include "config.h"

#include "stm_include/stm32/usart.h"

/*
 * Initializes the debug and configuration USART. This must be called
 * before any other function in this module (which includes the debug
 * funtions!).
 */
error_t debug_init() {
	USART1_BRR = CONSOLE_BAUD_VALUE;
	USART1_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

	return E_SUCCESS;
}

/*
 * Sends a character via the "debug TX" USART.
 */
void debug_putchar(const char message) {
	while (!(USART1_SR & USART_SR_TXE));
	USART1_DR = message;
	while (!(USART1_SR & USART_SR_TXE));
}

/*
 * Sends a message via the "debug TX" USART.
 */
void debug_write(const char *message, int length) {
	for (int i = 0; i < length; i++) {
		debug_putchar(message[i]);
	}
}

static unsigned int nybble(int x, int nybble) {
	int shift = 4 * nybble;
	return (x & (0xf << shift)) >> shift;
}

static char convert_hex(unsigned int x) {
	x = x & 0xf;
	if (x < 10) {
		return '0' + x;
	} else {
		return 'a' + (x - 10);
	}
}

/*
 * Converts the last "width" nybbles of x to hexadecimal format and
 * sends them via the debug USART.
 */
void debug_hex(int x, int width) {
	for (int i = width-1; i >= 0; i--) {
		debug_putchar(convert_hex(nybble(x, i)));
	}
}

/*
 * Converts the given integer to decimal format and sends it via the
 * debug USART. If the resulting string is shorter than min_width, it
 * is padded at the left with spaces.
 */
void debug_int(unsigned int x, int min_width) {
#define INT_WIDTH 10

	char buf[INT_WIDTH]; // Enough for 32 bit integers.
	int pos = INT_WIDTH;

	do {
		buf[--pos] = '0' + (x % 10);
		x /= 10;
	} while (x > 0);

	int number_len = INT_WIDTH - pos;
	for (int i = 0; i < min_width - number_len; i++) {
		debug_putchar(' ');
	}
	for (int i = pos; i < INT_WIDTH; i++) {
		debug_putchar(buf[i]);
	}
}

/*
 * Returns the next character received via the debug USART.
 * This function blocks until a character is available.
 *
 * If an error condition is signalled *during the wait*,
 * the function returns 0xff.
 */
char debug_getchar() {
	volatile char dump;
	
	while (!(USART1_SR & USART_SR_RXNE)) {
		if (USART1_SR & (USART_SR_ORE |
				 USART_SR_NE |
				 USART_SR_FE |
				 USART_SR_PE)) {
			// Must read data register to reset the error
			// condition.
			debug_hex(USART1_SR, 4);
			dump = USART1_DR;
			(void) dump;
			return 0xff;
		}
	}
	return USART1_DR;
}

/*
 * Reads a complete line into the given buffer.  The line may be at
 * most count characters long. Any further input except for "Return"
 * to finish the line is discarded and not echoed.
 */
void debug_getline(char *buffer, int count) {
	int i = 0; // Index where the next char is to be stored.
	char input;

	for (i = 0; i < count; i++) {
		buffer[i] = '\0';
	}

	i = 0;

	while(1) {
		input = debug_getchar();

		switch(input) {
		case '\n':
		case '\r':
			// Return is used to terminate the input.
			debug_string("\r\n");
			return;
		case '\b':
			if (i > 0) {
				i--;
				buffer[i] = '\0';
				debug_string("\b \b");
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
				debug_putchar(input);
				i++;
			} else {
				debug_string("X\b");
			}
			break;
		}
	}
}

// From http://forums.arm.com/index.php?/topic/13949-cycle-count-in-cortex-m3/
// These registers are undocumented officially...
volatile uint32_t * const DWT_CYCCNT  = (uint32_t*) 0xe0001004;
volatile uint32_t * const DWT_CONTROL = (uint32_t*) 0xe0001000;
volatile uint32_t * const SCB_DEMCR   = (uint32_t*) 0xe000edfc;

/*
 * Starts or restarts the cycle counter.
 */
inline void cycle_start() {
	*SCB_DEMCR |= 0x01000000;
	*DWT_CYCCNT = 0;
	*DWT_CONTROL |= 1;
}

/*
 * Returns the number of cycles elapsed since the cycle counter was
 * started or last restarted.
 */
inline int cycle_get() {
	return *DWT_CYCCNT;
}
