#include "console_wp.h"

#include <stdint.h>

#include "config.h"
#include "console.h"
#include "term.h"

/*
 * Functions for WP_ADJUST operation.
 */

/*
 * State to be held between commands
 */
static int selected_led = 0;

static uint16_t brightnesses[MODULE_LENGTH] = {[0 ... MODULE_LENGTH - 1] = 0x0000};

static const char *ROW_LED_INDEX  = "LED index:  ";
static const char *ROW_BRIGHTNESS = "Brightness: ";

static const char *HELP_LINE =
	"Select LED: a/d or h/l; Change line: TAB; Edit: e; Exit: q" CRLF
	"Adjust by 1: w/s or k/j; Adjust by 256: W/S or K/J";

/*
 * Displays the whitepoint and brightness status of the LEDs in the
 * WP_ADJUST input mask.
 */
void show_status_wp() {
	console_write(TERM_CLEAR);

	console_write(ROW_LED_INDEX);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		console_int_4d(i);
		console_write(" ");
	}

	console_write(CRLF CRLF);

	console_write(ROW_BRIGHTNESS);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (selected_led == i) {
			console_write(TERM_INVERT);
		}

		console_int_04x(brightnesses[i]);

		console_write(TERM_STANDARD " ");
	}

	console_write(CRLF CRLF);
	console_write(HELP_LINE);
}

static void inc16_unless_overflow(uint16_t *data, int16_t delta) {
	if ((delta < 0 && *data < -delta) ||
	    (delta > 0 && *data > 0xffff - delta)) {
		return;
	}

	*data += delta;
}

static void inc_selected(int16_t delta) {
	inc16_unless_overflow(&(brightnesses[selected_led]), delta);
}

static void run_direct_edit() {
	// First, place the cursor at the right value:
	// Reset to top left.
	console_write(TERM_CURSOR_RESET);
	// Move to correct line
	int down = 2;
	for (int i = 0; i < down; i++) {
		console_write("\n");
	}
	// Move to correct column
	int right = strlen(ROW_BRIGHTNESS) + selected_led * 5;
	for (int i = 0; i < right; i++) {
		console_write(TERM_CURSOR_RIGHT);
	}

	// Second, erase the value there.
	console_write("    ");
	console_write("\b\b\b\b");

	// Get the user input
	char input_line[4];
	unsigned int input;
	int pos = 0;

	console_getline(input_line, 4);
	if (parse_int(input_line, &pos, &input, 16) != E_SUCCESS ||
		input > 0xffff) {
		// If an error happens, ignore the input.
		return;
	}

	brightnesses[selected_led] = input;
}

static void leds_dark() {
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (pwm_set_brightness(i, 0) != E_SUCCESS) {
			error(ER_BUG, STR_WITH_LEN("Error while updating LEDs"), EA_PANIC);
		}
	}

	if (pwm_send_frame()) {
		error(ER_BUG, STR_WITH_LEN("Error while sending frame"), EA_PANIC);
	}
}

/*
 * Runs a command (i.e. keypress) in whitepoint adjust operation.
 *
 * Returns 1 if the console should exit and continue with normal mode.
 */
int run_command_wp() {
	char input = console_getchar();

	switch(input) {
	case 'q':
		console_write(TERM_CLEAR);
		console_set_operation(PROMPT);
		leds_dark();
		// Exit early here to avoid resetting the brightnesses to the stored
		// values.
		return 0;
		break;
	case 'd':
	case 'l':
		if (selected_led < MODULE_LENGTH - 1) {
			selected_led++;
		}
		break;
	case 'a':
	case 'h':
		if (selected_led > 0) {
			selected_led--;
		}
		break;
	case 'w':
	case 'k':
		inc_selected(1);
		break;
	case 'W':
	case 'K':
		inc_selected(0x100);
		break;
	case 's':
	case 'j':
		inc_selected(-1);
		break;
	case 'S':
	case 'J':
		inc_selected(-0x100);
		break;
	case 'e':
		run_direct_edit();
		break;
	default:
		break;
	}

	// Update all LEDs.
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (pwm_set_brightness(config.physical_led[i], brightnesses[i]) != E_SUCCESS) {
			error(ER_BUG, STR_WITH_LEN("Error while updating LEDs"), EA_PANIC);
		}
	}

	if (pwm_send_frame()) {
		error(ER_BUG, STR_WITH_LEN("Error while sending frame"), EA_PANIC);
	}

	return 0;
}
