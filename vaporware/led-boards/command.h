#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

#include "error.h"

/*
 * The two modes the program can be in.
 */
typedef enum {
	NORMAL_MODE,
	CONFIG_MODE
} vl_mode_t;

/*
 * Initializes the command module and sets up the USART filter
 * appropriately.
 */
void command_init();

/*
 * Returns the length of data (including the command) to be
 * expected for a given command code.
 *
 * set_mode must be called before this.
 */
int command_length(char command_code);

/*
 * Runs the command pointed to by 'command'. How the command is interpreted
 * depends on which mode the LED board is in.
 * Make sure that command has the right length (as indicated by command_length)!
 *
 * Returns an error/success code.
 */
error_t run_command(uint8_t *command);

#endif
