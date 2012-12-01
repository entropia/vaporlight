#include "led.h"

#include "config.h"
#include "gamma.h"

#include "stm_include/stm32/timer.h"

/*
 * Array of all the timer base addresses.
 */
#define TIMER_COUNT 6
static uint32_t TIMERS[TIMER_COUNT] = {
	TIM1,
	TIM2,
	TIM3,
	TIM15,
	TIM16,
	TIM17
};

/*
 * The raw PWM values for the LEDs.
 */
static uint16_t pwm_values[MODULE_LENGTH] = {
	[0 ... MODULE_LENGTH - 1] = 0x00
};

/*
 * Functions to manipulate one register in all the timers. Make sure that the
 * register in question is available in all timers (see defines in led.h).
 */
static void set_each(uint32_t reg_offset, int value) {
	for (int i = 0; i < TIMER_COUNT; i++) {
		TR(TIMERS[i], reg_offset) = value;
	}
}

static void or_each(uint32_t reg_offset, int value) {
	for (int i = 0; i < TIMER_COUNT; i++) {
		TR(TIMERS[i], reg_offset) |= value;
	}
}

/* Unused for now...
static void and_each(uint32_t reg_offset, int value) {
	for (int i = 0; i < TIMER_COUNT; i++) {
		TR(TIMERS[i], reg_offset) &= value;
	}
}
*/

/*
 * Enable all C/C outputs.
 */
static void enable_all() {
	// TODO: Can this be written as
	// 	set_each(CCER, TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);
	// or will that produce errors for TIM15-17?
	// The answer is: It is outside specification and should not be done.
	
	// Note inverted channels in TIM1!
	TR(TIM1 , CCER) = TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE | TIM_CCER_CC4E;
	TR(TIM2 , CCER) = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	TR(TIM3 , CCER) = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	TR(TIM15, CCER) = TIM_CCER_CC1E | TIM_CCER_CC2E;
	TR(TIM16, CCER) = TIM_CCER_CC1E;
	TR(TIM17, CCER) = TIM_CCER_CC1E;
}

/*
 * Disable all C/C outputs.
 */
static void disable_all() {
	set_each(CCER, 0);
}

/*
 * Initializes the LED module. This function must be called before
 * any other function in this module.
 */
void led_init() {
	// Initialize PWM.
	// Timer configuration should be:
	// upcounting
	// ARR = PWM_RELOAD
	// Send OCxREF to OCx output (CCxE = 1, CCxNE = 0)
	// PWM mode 1
	// ... the right register preloading magic.

	set_each(CR1, TIM_CR1_CKD_CK_INT | // Dead-time-clock = internal clock
		 TIM_CR1_CMS_EDGE |        // Edge mode
		 TIM_CR1_DIR_UP);          // Count up

	

	// Set all outputs to PWM mode 1.
	// Cannot use set_each here, because timers have
	// different numbers of channels.
	TR(TIM1 , CCMR1) = TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC1M_PWM1;
	TR(TIM1 , CCMR2) = TIM_CCMR2_OC4M_PWM1 | TIM_CCMR2_OC3M_PWM1;
	TR(TIM2 , CCMR1) = TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC1M_PWM1;
	TR(TIM2 , CCMR2) = TIM_CCMR2_OC4M_PWM1 | TIM_CCMR2_OC3M_PWM1;
	TR(TIM3 , CCMR1) = TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC1M_PWM1;
	TR(TIM3 , CCMR2) = TIM_CCMR2_OC4M_PWM1 | TIM_CCMR2_OC3M_PWM1;
	TR(TIM15, CCMR1) = TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC1M_PWM1;
	TR(TIM16, CCMR1) = TIM_CCMR1_OC1M_PWM1;
	TR(TIM17, CCMR1) = TIM_CCMR1_OC1M_PWM1;

	set_each(ARR, PWM_RELOAD);

	// Enable outputs:
	// 1. Set Main Output Enable (for those timers that have it)
	TR(TIM1 , BDTR) = TIM_BDTR_MOE;
	TR(TIM15, BDTR) = TIM_BDTR_MOE;
	TR(TIM16, BDTR) = TIM_BDTR_MOE;
	TR(TIM17, BDTR) = TIM_BDTR_MOE;
	// 2. Enable outputs
	enable_all();

	// Set the PWM values
	led_send_frame();

	// Force the registers to be actually loaded.
	or_each(EGR, TIM_EGR_UG);

	// Finally enable the timers.
	or_each(CR1, TIM_CR1_CEN);
}


/*
 * Sets the state of all LEDs and the PWM hardware. For available states
 * see led_state_t.
 */
void led_set_state(led_state_t state) {
	// The order of the following two if-blocks is for safety reasons:
	// If both LED_ON and LED_OFF are set, the LEDs will end up off.
	
	if (state & LED_ON) {
		// Switch on PWM modules
		// Simply reset CCR values to their previous state.
		enable_all();
	}

	if (state & LED_OFF) {
		// Switch off PWM modules
		disable_all();
	}

	if (state & LED_ZERO) {
		for (unsigned int l = 0; l < MODULE_LENGTH; l++) {
			led_set_brightness(l, 0);
		}
	}
}


/*
 * Sets the brightness value for the given LED to the given value.
 * The value is immediately calibrated, but only stored to be sent
 * to the PWM module later.
 *
 * Returns an error/success code.
 */
error_t led_set_brightness(uint8_t led, uint8_t brightness) {
	if (led >= MODULE_LENGTH) {
		error(ER_BUG, STR_WITH_LEN("LED index out of range"), EA_RESUME);
		return E_INDEXRANGE;
	} else {
		pwm_values[led] = (gamma(config.led_color[led], brightness) *
				   config.white_correction[led])
					>> 16;
		return E_SUCCESS;
	}
}

/*
 * Sends the status of all LEDs to the PWM registers.
 *
 * Returns an error/success code.
 */
error_t led_send_frame() {
	for (int i = 0; i < MODULE_LENGTH; i++) {
		*TIMER_CHANNELS[i] = pwm_values[i];
	}

	return E_SUCCESS;
}
