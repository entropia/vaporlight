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
 * the timer base addresses (which are taken from the libopenstm32 headers).
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
 * The states the LEDs can be set to using led_set_state.
 *     LED_OFF:     Disables PWM output (hardware).
 *     LED_ON:      Enables PWM output (hardware).
 *     LED_ZERO:    Sets all brightnesses to 0.
 *     LED_STOP:    Sets all brightnesses to 0 and disables PWM output.
 *     LED_START:   Alias for LED_ON
 */
typedef enum {
	LED_OFF = 0x01,
	LED_ON = 0x02,
	LED_ZERO = 0x04,

	LED_STOP = LED_OFF | LED_ZERO,
	LED_START = LED_ON
} led_state_t;


/*
 * Initializes the LED module. This function must be called before
 * any other function in this module.
 */
void led_init();

/*
 * Sets the state of all LEDs and the PWM hardware. For available states
 * see led_state_t.
 */
void led_set_state(led_state_t state);

/*
 * Sets the brightness value for the given LED to the given value.
 * The value is immediately calibrated, but only stored to be sent
 * to the PWM module later.
 *
 * Returns an error/success code.
 */
error_t led_set_brightness(uint8_t led, uint8_t brightness);

/*
 * Sends the status of all LEDs to the PWM registers.
 *
 * Returns an error/success code.
 */
error_t led_send_frame();

#endif
