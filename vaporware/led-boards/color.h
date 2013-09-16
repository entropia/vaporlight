#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

#include "config.h"

/*
 * Performs color correction accoring to the given led_info_t.
 *
 * This function converts the color given in x, y, Y to the
 * appropriate PWM channel settings for the LED with the given
 * led_info_t. The settings are returned through *r, *g, *b.
 */
void color_correct(led_info_t info,
		   uint16_t x, uint16_t y, uint16_t Y,
		   uint16_t rgb[static 3]);

/*
 * Converts a flatly counted channel index (i.e. 0 to 15 standing for
 * LED0 red, LED0 green, LED0 blue, LED1 red, ...) to the PWM channel
 * index to use for this LED.
 */
uint8_t convert_channel_index(uint8_t c);

/*
 * Writes the inverse of the 3x3 matrix pointed to by in to the 3x3
 * matrix pointed to by out.
 */
void invert_3x3(fixed_t in[static 9], fixed_t out[static 9]);

#endif
