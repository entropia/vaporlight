/*
 * Low-level module for the debug and configuration USART (USART1).
 *
 * For simplicity, all the public functions in here switch off
 * interrupts.
 */
#include "usart1.h"

#include <stdint.h>

#include "config.h"
#include "error.h"
#include "pwm.h"
#include "sync.h"
#include "term.h"

#include "stm_include/stm32/nvic.h"
#include "stm_include/stm32/usart.h"

/***************************************
 * Output side
 */

#define OUTPUT_BUFFER_LENGTH 80
static char output_buffer[OUTPUT_BUFFER_LENGTH];
static int output_write_index = 0;
static int output_read_index = 0;

// The number of characters still to be sent.  Not strictly necessary,
// but makes code easier to understand.  This must be volatile,
// because it is accessed concurrently in usart1_putchar.
static volatile int output_pending = 0;

// Set to 1 by the ISR when the USART is done and no character is left
// in the buffer. Then, the next character must not be enqueued but sent directly.
static int output_idle = 0;

// When this is not '\0', it holds a character that must urgently be
// sent via the USART (this is used for flow control).
static char urgent_send = '\0';

/*
 * Sends a character via USART1.
 */
void usart1_putchar(const char message) {
	// Before going atomic, make sure there is space in the
	// output buffer.  Since the ISR only decreases
	// output_pending, no double checked locking is necessary.
	while (output_pending >= OUTPUT_BUFFER_LENGTH) {
		if (!(USART1_CR1 & USART_CR1_TXEIE)) {
			error(ER_BUG,
			      STR_WITH_LEN("TXEIE lost"),
			      EA_PANIC);
		}
	}

	interrupts_off();

	output_buffer[output_write_index++] = message;
	if (output_write_index == OUTPUT_BUFFER_LENGTH) {
		output_write_index = 0;
	}

	output_pending++;

	if (output_idle) {
		output_idle = 0;
		USART1_CR1 |= USART_CR1_TXEIE;
	}

	interrupts_on();

}

static void isr_ready_to_send() {
	if (urgent_send != '\0') {
		USART1_DR = urgent_send;
		urgent_send = '\0';

	} else if (output_pending > 0) {
		USART1_DR = output_buffer[output_read_index++];
		if (output_read_index == OUTPUT_BUFFER_LENGTH) {
			output_read_index = 0;
		}

		output_pending--;

	} else /* output_pending == 0 */ {
		// Switch off TXE interrupt to avoid infinite
		// interrupt loop.
		USART1_CR1 &= ~(USART_CR1_TXEIE);

		output_idle = 1;
	}
}

/*
 * If the USART is idle, sends the given message. Otherwise, sets
 * urgent_send to the message. The previous content of urgent_send
 * will be overwritten if it has not yet been sent.
 */
static void usart1_urgent_send(char message) {
	urgent_send = message;

	if (output_idle) {
		output_idle = 0;
		USART1_CR1 |= USART_CR1_TXEIE;
	}
}

/***************************************
 * Input side
 */

#define INPUT_BUFFER_LENGTH 50
#define XOFF_TRESHOLD 40
#define XON_TRESHOLD 10

static char input_buffer[INPUT_BUFFER_LENGTH];

static int input_write_index = 0;
static int input_read_index = 0;
static volatile int input_pending = 0;

// 1, if an XON has been sent for the current buffer underflow.
static int xon_sent = 0;

static void isr_read_input(char input) {
	if (input_pending >= INPUT_BUFFER_LENGTH) {
		// Flow control has failed.
		error(ER_BUG,
		      STR_WITH_LEN("Input buffer overflow (flow control failed)"),
		      EA_PANIC);
	}

	input_buffer[input_write_index++] = input;
	if (input_write_index == INPUT_BUFFER_LENGTH) {
		input_write_index = 0;
	}

	input_pending++;

	if (input_pending > XOFF_TRESHOLD) {
		// This repeatedly sends XOFF when more data is
		// coming. I cannot see any harm in that.
		usart1_urgent_send(CHAR(XOFF));
	}
}

/*
 * Returns the next character received via the debug USART.
 * This function blocks until a character is available.
 */
char usart1_getchar() {
	while(input_pending <= 0)
		;

	interrupts_off();

	char result = input_buffer[input_read_index++];
	if (input_read_index == INPUT_BUFFER_LENGTH) {
		input_read_index = 0;
	}

	input_pending--;

	if (input_pending < XON_TRESHOLD) {
		if (!xon_sent) {
			usart1_urgent_send(CHAR(XON));
			xon_sent = 1;
		}
	} else {
		xon_sent = 0; // Reset for the next underrun.
	}

	interrupts_on();

	return result;
}

/*
 * Return 1 if there is a character to read and 0 otherwise.
 *
 * Note that due to concurrency, the return value may be outdated by
 * the time you get it,
 */
int usart1_has_input() {
	return (input_pending > 0);
}

/***************************************
 * Initialization and the ISR
 */

void usart1_init() {
	USART1_BRR = CONSOLE_BAUD_VALUE;

	USART1_CR1 = USART_CR1_UE |
		USART_CR1_TE | USART_CR1_TXEIE |
		USART_CR1_RE | USART_CR1_RXNEIE;

	NVIC_ISER(1) |= (1 << (NVIC_USART1_IRQ - 32));
}

void __attribute__ ((interrupt("IRQ"))) isr_usart1() {
	unsigned short sr = USART1_SR;

	if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
		error(ER_USART_RX, STR_WITH_LEN("USART1 chrashed"), EA_PANIC);
	}

	unsigned short dr = USART1_DR; // This read resets the error flags.

	if (sr & USART_SR_TXE) {
		isr_ready_to_send();
	}
	if (sr & USART_SR_RXNE) {
		isr_read_input((char)dr);
	}
}
