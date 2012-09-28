#ifndef USART_H
#define USART_H

/*
 * This modules manages the RS485 USART (USART2) which is used to
 * receive commands in normal mode. Note that USART1 is completely
 * managed by the console module.
 */

#include <stdint.h>

#include "command.h"

/*
 * The type of filter function that can be passed to set_usart_filter.
 */
typedef uint8_t ((*usart_filter_t)(uint8_t));

/*
 * Initializes the RS485 bus USART. This must be called before any other
 * function accessing the USART.
 */
error_t usart_init();

/*
 * Returns a pointer to a buffer containing the next available USART command that
 * the current filter is interested in.
 * If no command is available, returns NULL.
 */
unsigned char *usart_next_command();

/*
 * Sets a command filter function for USART reception.
 * The filter gets passed a USART command and must return a nonzero value
 * to indicate interest in the command.
 */
void usart_set_filter(usart_filter_t filter);

/*
 * ISR for USART1.
 */
void isr_usart1();

/*
 * ISR for USART2.
 */
void isr_usart2();

#endif
