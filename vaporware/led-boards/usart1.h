#ifndef USART1_H
#define USART1_H

/*
 * Initializes the debug and configuration USART. This must be called
 * before any other function in this module (which includes the debug
 * funtions!).
 */
void usart1_init();

/*
 * Sends a character via USART1.
 */
void usart1_putchar(const char message);

/*
 * Returns the next character received via the debug USART.
 * This function blocks until a character is available.
 */
char usart1_getchar();

/*
 * Return 1 if there is a character to read and 0 otherwise.
 *
 * Note that due to concurrency, the return value may be outdated by
 * the time you get it,
 */
int usart1_has_input();

/*
 * ISR for usart1.
 */
void isr_usart1();

#endif
