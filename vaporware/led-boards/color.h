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
void color_correct(led_info_t *info,
		   uint16_t x, uint16_t y, uint16_t Y,
		   uint16_t *r, uint16_t *g, uint16_t *b);

#endif
