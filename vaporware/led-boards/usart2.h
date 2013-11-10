#ifndef USART_H
#define USART_H

/*
 * This modules manages the RS485 USART (USART2) which is used to
 * receive commands in normal mode. Note that USART1 is completely
 * managed by the console module.
 */

#include <stdbool.h>
#include <stdint.h>

#include "command.h"

/*
 * The type of filter function that can be passed to usart2_set_address_filter.
 */
typedef bool ((*usart_address_filter_t)(uint8_t));

/*
 * The type of function that can be passed to usart2_set_length_check.
 */
typedef int ((*usart_length_check_t)(uint8_t*, int));

/*
 * Initializes the RS485 bus USART. This must be called before any other
 * function accessing the USART.
 */
void usart2_init();

/*
 * Returns a pointer to a buffer containing the next available USART command that
 * the current filter is interested in.
 * If no command is available, returns NULL.
 */
unsigned char *usart2_next_command();

/*
 * Sets a command filter function for USART reception.
 * The filter gets passed a USART command and must return a nonzero value
 * to indicate interest in the command.
 */
void usart2_set_address_filter(usart_address_filter_t filter);

/*
 * Sets a length check function for USART reception.
 *
 * After the first byte of a command (after the address byte) is
 * received by the ISR, the length check function is called to
 * determine, if the command currently received is complete.

 * The length check function must return the minimum number of bytes
 * which need to be received to make a complete command. It will be
 * called again after this number of bytes has been received.
 *
 * Since timing is critical in the ISR, the length check function
 * should parse the command as little as possible, and only ascertain
 * its required length.
 */
void usart2_set_length_check(usart_length_check_t length_check);

/*
 * ISR for USART2.
 */
void isr_usart2();

#endif
