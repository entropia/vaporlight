#ifndef CONSOLE_PROMPT_H
#define CONSOLE_PROMPT_H

/*
 * This module contains the implementations of the commands available
 * on the configuration console in prompt operation.
 */

/*
 * Displays the current configuration and a prompt on the debug
 * console.
 */
void show_status_prompt();

/*
 * Reads a command entered on the debug console and executes it
 * according to the commands array.
 *
 * Returns 1 if the console should exit and continue with normal mode.
 */
int run_command_prompt();

#endif
