#include "usart.h"

#include "command.h"
#include "config.h"
#include "error.h"
#include "fail.h"
#include "macros.h"

#if defined COUNT_USART_ISR || defined TRACE_USART
	#include "debug.h"
#endif

#include "stm_include/stm32/nvic.h"
#include "stm_include/stm32/usart.h"

/*
 * The possible states that the USART ISR can be in.
 */
typedef enum {
	// The USART is not currently reading a command.
	IDLE,

	// A start byte (0x55) has been received.
	GOT_START,

	// A command byte that the command interpreter is interested
	// in has been received. Its arguments are now read.
	READING,
} isr_state_t;

/*
 * The currently set filter for USART commands.
 */
static usart_filter_t usart_filter;

/*
 * Shared between ISR and usart_next_command.
 * Remember to synchronize!
 *
 * commands_pending is incremented by the ISR when a command has been fully
 * received and stored. If usart_next_command finds commands_pending to be
 * greater than 0, a command is available at *read_buffer. After processing
 * this command, usart_next_command advances read_buffer by one buffer and
 * decrements commands_pending. If commands_pending equals USART_BUFFER_COUNT,
 * the ISR may not store another command (the command will be dropped), because
 * all buffers already contain a command to be read by usart_next_command.
 */
volatile static int commands_pending = 0;

/*
 * Also shared between ISR and usart_next_command.
 *
 * This is set by the ISR to indicate that a command could not be stored because
 * no command buffer was free. It is checked by the next call to
 * usart_next_command and an error message is sent when overflow has occurred.
 */
volatile static int command_overflow = 0;

// A ring buffer for concurrent read and write.
static unsigned char isr_buffers[USART_BUFFER_COUNT][CMD_BUFFER_LEN];

// The buffer currently read from.
static int read_buffer = 0;

// 1, if the "outside" is currently reading from read_buffer.
static int is_reading = 0;

/*
 * Variables only to be used by the ISR and helper functions.
 * All these variables apply only to the USART that is currently
 * in receiving mode (USART1 in config mode, USART2 in normal mode).
 */
// Current state of the receiving USART.
static isr_state_t isr_state = IDLE;
// 1, if the last character was ESCAPE_MARK
static int isr_escape = 0;
// The buffer currently used for writing.
static int isr_write_buffer = 0;
// Index into the writing buffer, points to first free space.
static int isr_write_idx = 0;
// Total number of bytes to be written for this command.
static int isr_length = 0;
// USART failure counter.
static fail_t isr_usart_fails;

/*
 * Initializes the RS485 bus USART. This must be called before any other
 * function accessing the USART.
 */
error_t usart_init() {
	// Warning: Heuristic!
	// Even if these checks do not fail, the USART buffers may still be too short.
	// They must be large enough to store the longest possible command.
	// For command lengths, see command_len in command.c.
	_Static_assert(CMD_BUFFER_LEN > MODULE_LENGTH + 3, "USART buffers are too short");
	_Static_assert(CMD_BUFFER_LEN > 4, "USART buffers are too short");

	fail_init(&isr_usart_fails, USART_FAIL_TRESHOLD);

	USART2_BRR = USART_BAUD_VALUE;

	USART2_CR1 = USART_CR1_UE |
		USART_CR1_RXNEIE |
		USART_CR1_RE;

	NVIC_ISER(1) |= (1 << (NVIC_USART2_IRQ - 32));

	return E_SUCCESS;
}

/*
 * Increments the value pointed to by ptr atomically.
 */
static void atomic_increment(volatile int *ptr) {
	__sync_fetch_and_add(ptr, 1);
}

/*
 * Decrements the value pointed to by ptr atomically.
 */
static void atomic_decrement(volatile int *ptr) {
	__sync_fetch_and_sub(ptr, 1);
}

/*
 * Sets the value pointed to by ptr to 1.
 * The value must have been 1 or 0 before.
 */
static void atomic_set(volatile int *ptr) {
	__sync_fetch_and_or(ptr, 1);
}

/*
 * Sets the value pointed to by ptr to 0.
 * The value must have been 1 or 0 before.
 */
static void atomic_reset(volatile int *ptr) {
	__sync_fetch_and_and(ptr, 0);
}

/*
 * Returns a pointer to a buffer containing the next available USART command that
 * the current filter is interested in.
 * If no command is available, returns NULL.
 */
unsigned char *usart_next_command() {
	// 1. Free the last command if it was being read from.
	if (is_reading) {
		if (read_buffer == USART_BUFFER_COUNT - 1) {
			read_buffer = 0;
		} else {
			read_buffer++;
		}
		atomic_decrement(&commands_pending);

		is_reading = 0;
	}

	// 2. Check if an overflow needs to be reported.
	if (command_overflow) {
		error(ER_CMDOVERFLOW, STR_WITH_LEN("CO"), EA_RESUME);
		atomic_reset(&command_overflow);
	}

	// 3. Check if there is a new command.
	// Note that this section need not be atomic, because even if
	// an interrupt occurs here, it can only append to the
	// command buffer and never overwrite an existing command.
	if (commands_pending > 0) {
		is_reading = 1;
		return &(isr_buffers[read_buffer][0]);
	} else {
		return (unsigned char*) 0;
	}
}

/*
 * Sets a command filter function for USART reception.
 * The filter gets passed a USART command and must return a nonzero value
 * to indicate interest in the command.
 */
void usart_set_filter(usart_filter_t filter) {
	usart_filter = filter;
}

/*
 * Functions for the ISR (and the ISR itself).
 */

/*
 * Handles an error on the USART by discarding the unfinished command and
 * recording the failure.
 */
static void isr_read_error() {
	dled_toggle();
	
	// Abort current reception
	isr_write_idx = 0;
	isr_state = IDLE;

	if (fail_event(&isr_usart_fails, 1)) {
		// Too many failures, raise an error.
		error(ER_USART_RX, STR_WITH_LEN("Too many USART errors"), EA_RESET);
	}
}

/*
 * Checks to see if the command currently in the active ISR write buffer is
 * finished. If so, moves on to the next buffer and notifies the main program.
 */
static void isr_try_finish() {
	if (isr_write_idx == isr_length) {
#ifdef TRACE_USART
			debug_string("f");
#endif

		// Command is finished.
		if (isr_write_buffer == USART_BUFFER_COUNT - 1) {
			isr_write_buffer = 0;
		} else {
			isr_write_buffer++;
		}
		isr_write_idx = 0;
		isr_state = IDLE;

		atomic_increment(&commands_pending);
	}
}

/*
 * Adds the given byte to the command that is currently assembled. If
 * the byte finishes a command, the command is made available to
 * usart_next_command.
 */
static void isr_read_command(unsigned char in_byte) {
	// If the last character was ESCAPE_MARK, we now need
	// to evaluate the escape sequence.
	if (isr_escape) {
		isr_escape = 0;

		switch (in_byte) {
		case 0x00:
			in_byte = 0x54;
			break;
		case 0x01:
			in_byte = 0x55;
			break;
		default:
			// Bad escape sequence
			isr_read_error();
			return;
			break;
		}
	}

	// If ESCAPE_MARK is received, it must be dropped and
	// only isr_escape set.
	if (in_byte == ESCAPE_MARK) {
		isr_escape = 1;
		return;
	}
	
	// Whatever the state is, on receiving the start-of-command
	// mark we must abort the current command and go into
	// the GOT_START state.
	if (in_byte == START_MARK) {
		isr_write_idx = 0;
		isr_state = GOT_START;
	} else {

		switch(isr_state) {
		case IDLE:
			// Nothing to do. The code above takes care of
			// receiving the start byte.
			break;
		
		case GOT_START:		
			// This byte (the one after start) is the command.
			// Check if the command interpreter is interested.
			if (usart_filter(in_byte)) {
				// Start a new command.
				if (commands_pending >= USART_BUFFER_COUNT) {
					// There are no free command buffers,
					// this command must be dropped.
					atomic_set(&command_overflow);
					isr_state = IDLE;
					return;
				}

				isr_state = READING;
				isr_buffers[isr_write_buffer][isr_write_idx++] = in_byte;
				isr_length = command_length(in_byte);

				if (isr_length == 0) {
					// An invalid command was received.
					// Abort the command.
					isr_write_idx = 0;
					isr_state = IDLE;
				} else {
					// The command is valid, in fact it could already
					// be finished (such as Strobe). Check for that.
					isr_try_finish();
				}

			} else {
				isr_state = IDLE;
			}
			break;
		
		case READING:
			// Next byte.
			isr_buffers[isr_write_buffer][isr_write_idx++] = in_byte & 0xff;
			isr_try_finish();
			break;

		default:
			error(ER_BUG, STR_WITH_LEN("isr_read_command: Corrupt isr_state"),
			      EA_PANIC);
		}
	}
}

/*
 * Dispatches a USART interrupt. Arguments should be the status and
 * data register of the USART which caused the interrupt.
 */
void isr_dispatch(unsigned short sr, unsigned short dr) {
	// First, check for errors.
	if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
		isr_read_error();
	} else if (sr & USART_SR_RXNE) {
		fail_event(&isr_usart_fails, 0);
		isr_read_command(dr & 0xff);
	} else {
		error(ER_USART_RX, STR_WITH_LEN("Strage interrupt on USART."), EA_RESET);
	}
}

/*
 * ISR for USART2.
 */
void __attribute__ ((interrupt("IRQ"))) isr_usart2() {
#ifdef COUNT_USART_ISR
	cycle_start();
#endif

	unsigned short sr = USART2_SR;
	unsigned short dr = USART2_DR;

	isr_dispatch(sr, dr);

#ifdef COUNT_USART_ISR
	int cycles = cycle_get();
	debug_string("UC2");
	debug_write((char*) &cycles, 4);
#endif

}
