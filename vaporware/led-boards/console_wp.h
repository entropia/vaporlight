#ifndef CONSOLE_WP_H
#define CONSOLE_WP_H

/*
 * Displays the whitepoint and brightness status of the LEDs in the
 * WP_ADJUST input mask.
 */
void show_status_wp();

/*
 * Runs a command (i.e. keypress) in whitepoint adjust operation.
 *
 * Returns 1 if the console should exit and continue with normal mode.
 */
int run_command_wp();

#endif
