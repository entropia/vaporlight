/*
  This is the main program file for the Vaporlight Forever LED board controller.
*/
#include "main.h"

#include "color.h"
#include "command.h"
#include "config.h"
#include "console.h"
#include "debug.h"
#include "error.h"
#include "fail.h"
#include "git_version.h"
#include "heat.h"
#include "pwm.h"
#include "usart1.h"
#include "usart2.h"

/*
 * Set to 1 by the systick timer when a heat check
 * should be done.
 */
volatile int do_heat_check;

/*
 * What to do when overheat is detected.
 */
static void panic_on_overheat() {
	pwm_set_state(PWM_STOP);
	error(ER_HEAT, STR_WITH_LEN("Overheat!"), EA_RESET);
}

/*
 * Display the current git revision as colors
 */
static void display_version(void) {
	uint32_t rev = GIT_VERSION_HEX;

	for(uint8_t c = 0; c < 15; c++) {
		uint8_t pwm_channel = convert_channel_index(c);

		pwm_set_brightness(pwm_channel, (rev & 1) ? 0xFFFF : 0x0);

		rev >>= 1;
	}
}

/*
 * Main program. Initializes all the modules and then enters the main loop.
 * The actual work is done in command.c.
 */
int main() {
	unsigned char *command;
	error_t ret;
	vl_mode_t mode;

	usart1_init();

#ifdef TRACE_STARTUP
	dled_off();
#endif

	pwm_init();

	heat_init(panic_on_overheat);

	ret = load_config();

	// Display the version number on lights during bootup
	if(ret == E_SUCCESS)
		display_version();

	mode = console_ask_mode();

	pwm_set_state(PWM_STOP);

	if (ret == E_SUCCESS) {
		// We are running in normal mode with config available.
		// That's OK.
	} else if (ret == E_NOCONFIG && mode != CONFIG_MODE) {
		error(ER_NO_CONFIG, STR_WITH_LEN("No configuration found"), EA_PANIC);
	} else if (ret != E_NOCONFIG && ret != E_SUCCESS) {
		error(ER_BUG, STR_WITH_LEN("Flash read error"), EA_PANIC);
	} else {
		// We are running in config mode with no config available.
		// That's OK.
	}

	// Enable interrupts for heat check.
	__asm("cpsie i");

	pwm_set_state(PWM_ON);

	// If we are running in config mode, first start the config
	// console.  The user may choose to continue running the board
	// after configuration is done, so we must be able to continue
	// as if in normal mode.
	if (mode == CONFIG_MODE) {
		console_run();
	}

	// We are now, regardless of the value of mode, in normal mode.

	command_init();
	usart2_init();

	while (1) {
		command = usart2_next_command();

		if (command != (unsigned char*) 0) {
#ifdef TRACE_COMMANDS
			debug_string("C:");
#endif
			ret = run_command(command);

			if (ret != E_SUCCESS) {
				error(ER_USART_RX, STR_WITH_LEN("Bogus USART command."), EA_RESUME);
			}
		}

		if (do_heat_check) {
#ifdef TRACE_HEAT
			debug_string("H\n");
#endif
			heat_timer_tick();

			do_heat_check = 0;
		}
	}

	error(ER_BUG, STR_WITH_LEN("End of main reached!"), EA_PANIC);

	return 0;
}
