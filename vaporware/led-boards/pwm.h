#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "stm_include/stm32/timer.h"

#include "error.h"

/*
 * Since we must control many timers at once in here, we use some slightly
 * different register definitions from the libopenstm32 header files:
 * Since we need a way of programmatically accessing the same register for
 * several timers, we have a separate definitio of the register offsets and
 * the timer base addresses (which are taken from the libopencm3 headers).
 */

/*
 * Generic timer register map.
 */
                   // Register is present in:
#define CR1   0x00 // TIM1 TIM2-3 TIM15 TIM16-17
#define CR2   0x04 // TIM1 TIM2-3 TIM15 TIM16-17
#define SMCR  0x08 // TIM1 TIM2-3 TIM15
#define DIER  0x0c // TIM1 TIM2-3 TIM15 TIM16-17
#define SR    0x10 // TIM1 TIM2-3 TIM15 TIM16-17
#define EGR   0x14 // TIM1 TIM2-3 TIM15 TIM16-17
#define CCMR1 0x18 // TIM1 TIM2-3 TIM15 TIM16-17
#define CCMR2 0x1c // TIM1 TIM2-3
#define CCER  0x20 // TIM1 TIM2-3 TIM15 TIM16-17
#define CNT   0x24 // TIM1 TIM2-3 TIM15 TIM16-17
#define PSC   0x28 // TIM1 TIM2-3 TIM15 TIM16-17
#define ARR   0x2c // TIM1 TIM2-3 TIM15 TIM16-17
#define RCR   0x30 // TIM1        TIM15 TIM16-17
#define CCR1  0x34 // TIM1 TIM2-3 TIM15 TIM16-17
#define CCR2  0x38 // TIM1 TIM2-3 TIM15
#define CCR3  0x3c // TIM1 TIM2-3
#define CCR4  0x40 // TIM1 TIM2-3
#define BDTR  0x44 // TIM1        TIM15 TIM16-17
#define DCR   0x48 // TIM1 TIM2-3 TIM15 TIM16-17
#define DMAR  0x4c // TIM1 TIM2-3 TIM15 TIM16-17

/*
 * Constructs a timer register. The macro MMIO32 comes from
 * stm_include/cm3/common.h
 */
#define TR(timer, reg) MMIO32((timer) + (reg))

/*
 * Enum for the LED colors available. The values correspond to those
 * used in USART communication and indices into the gamma table list.
 */
typedef enum {
	RED = 0,
	GREEN = 1,
	BLUE = 2,
	WHITE = 3
} color_t;

/*
 * The states the PWM can be set to using pwm_set_state.
 *     PWM_OFF:     Disables PWM output (hardware).
 *     PWM_ON:      Enables PWM output (hardware).
 *     PWM_ZERO:    Sets all brightnesses to 0.
 *     PWM_STOP:    Sets all brightnesses to 0 and disables PWM output.
 *     PWM_START:   Alias for PWM_ON
 */
typedef enum {
	PWM_OFF = 0x01,
	PWM_ON = 0x02,
	PWM_ZERO = 0x04,

	PWM_STOP = PWM_OFF | PWM_ZERO,
	PWM_START = PWM_ON
} pwm_state_t;


/*
 * Initializes the PWM module. This function must be called before
 * any other function in this module.
 */
void pwm_init();

/*
 * Sets the state of all PWM channels and the PWM hardware. For available states
 * see pwm_state_t.
 */
void pwm_set_state(pwm_state_t state);

/*
 * Sets the brightness value for the given PWM channel to the given
 * value.  The value is only stored to be sent to the PWM module
 * later.
 *
 * Returns an error/success code.
 */
error_t pwm_set_brightness(uint8_t led, uint8_t brightness);

/*
 * Sends the status of all PWM channels to the hardware PWM registers.
 *
 * Returns an error/success code.
 */
error_t pwm_send_frame();

#endif
