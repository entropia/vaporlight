/*
 * This module access to the debug USART and a few other debug-related
 * functions.
 */

#include "debug.h"

#include <stdint.h>

#include "config.h"
#include "console.h"

#include "stm_include/stm32/usart.h"

/*
 * Sends a character via the "debug TX" USART.
 */
void debug_putchar(const char message) {
#ifndef NDEBUG
	console_putchar(message);
#endif
}

/*
 * Sends a message via the "debug TX" USART.
 */
void debug_write(const char *message, int length) {
#ifndef NDEBUG
	console_write_raw(message, length);
#endif
}

/*
 * Converts the last "width" nybbles of x to hexadecimal format and
 * sends them via the debug USART.
 */
void debug_hex(int x, int width) {
#ifndef NDEBUG
	console_int(x, 16, width, '0');
#endif
}

/*
 * Converts the given integer to decimal format and sends it via the
 * debug USART. If the resulting string is shorter than min_width, it
 * is padded at the left with spaces.
 */
void debug_int(unsigned int x, int min_width) {
#ifndef NDEBUG
	console_int(x, 10, min_width, ' ');
#endif
}

void debug_fixed(fixed_t x) {
#ifndef NDEBUG
	if (fixlt(x, FIXNUM(0))) {
		console_write("-");
		x = fixneg(x);
	}
	console_int(fix_int_part(x), 10, 0, ' ');
	console_write(".");
	console_int(fix_fract_part(x), 10, 0, ' ');
#endif
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
