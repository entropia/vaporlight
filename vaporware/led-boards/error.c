#include "error.h"

#include "config.h"
#include "console.h"
#include "debug.h"
#include "led.h"

#include "stm_include/stm32/gpio.h"
#include "stm_include/stm32/scb.h"

/*
 * Switches on the debug LED.
 */
inline void dled_on() {
	GPIOC_ODR |= GPIO12;
}

/*
 * Switches off the debug LED.
 */
inline void dled_off() {
	GPIOC_ODR &= ~GPIO12;
}

/*
 * Toggles the debug LED.
 */
void dled_toggle() {
	GPIOC_ODR ^= GPIO12;
}

/*
 * Delay loop.
 */
static void delay_ms(int ms) {
#ifdef SHORT_LOOPS
	for (int i = 0; i < 3; i++) {
#else
	for (int i = 0; i < ms * 6000; i++) {
#endif
		__asm("nop");
	}
}

/*
 * Blinks the debug LED in the pattern given by interpreting it
 * bitwise (most-significant-bit first). If the bit is 1, the debug
 * LED is switched on, if it is 0, the debug LED is switched off.
 */
static void dled_blink(int pattern) {
	dled_off();

	delay_ms(DEBUG_LED_SPEED);
	
	for (int i = 8 * sizeof(int); i >= 0; i--) {
		if (pattern & (1 << i)) {
			dled_on();
		} else {
			dled_off();
		}

		delay_ms(DEBUG_LED_SPEED);
	}
}

/*
 * Reports an error. reason should roughly give the reason for the error. Further
 * details can be supplied in the message (which may contain null bytes and whose
 * length is indicated by length). action describes which actions to take after the
 * error has been reported.
 */
void error(err_reason_t reason, char *message, int length, err_action_t action) {
	switch(action) {
	case EA_RESUME:
		dled_toggle();
#ifdef TRACE_ERRORS
		dled_blink((int) reason);
		debug_write(message, length);
#endif
		return;
		break;
	case EA_RESET:
		led_set_state(LED_STOP);
		
		dled_blink((int) reason);
		console_write_raw(message, length);
		
		// Reset the system
		SCB_AIRCR |= SCB_AIRCR_SYSRESETREQ;
		break;
	case EA_PANIC:
		led_set_state(LED_STOP);
		
		while (1) {
			dled_blink((int) reason);
			console_write_raw(message, length);
		}
		
		break;
	}
}
