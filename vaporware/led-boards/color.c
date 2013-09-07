#include "color.h"

#include "console.h"
#include "term.h"

/*
 * This module currently uses floating point numbers internally. They
 * should be converted once the algorithm is stable.
 */

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void mat_x_vec(float m[static 9], float x[static 3], float result[static 3]) {
	result[0] = m[0] * x[0] + m[1] * x[1] + m[2] * x[2];
	result[1] = m[3] * x[0] + m[4] * x[1] + m[5] * x[2];
	result[2] = m[6] * x[0] + m[7] * x[1] + m[8] * x[2];
}

static float dot(float a[static 3], float b[static 3]) {
	return a[0] * b[0] +
		a[1] * b[1] +
		a[2] * b[2];
}

static float clamp(float x, float min, float max) {
	if (x < min) {
		return min;
	} else if (x > max) {
		return max;
	} else {
		return x;
	}
}

/*
 * Performs color correction according to the given led_info_t.
 *
 * This function converts the color given in x, y, Y to the
 * appropriate PWM channel settings for the LED with the given
 * led_info_t. The settings are returned through *r, *g, *b.
 */
void color_correct(led_info_t *info,
		   uint16_t x, uint16_t y, uint16_t Y,
		   uint16_t rgb[static 3]) {

	// First, get the ratio of the PWM channels right.  This is
	// done by finding the barycentric coordinates of xyY within
	// the LED's gamut.
	// info already contains the inverted matrix
	// necessary to compute this.
	float in[3] = {
		(x * 1.0f) / 0xffff,
		(y * 1.0f) / 0xffff,
		1.0f
	};
	float rgb_ratio[3];

	mat_x_vec(info->color_matrix, in, rgb_ratio);

	// Now adjust for the desired luminosity.
	// Note that the color takes precedence: If reproducing the
	// color at the requested luminosity it not possible, it will
	// be reproduced darker.
	float total_Y = dot(rgb_ratio, info->peak_Y);
	// The maximum possible scaling factor that will still keep
	// the color.  (Actually the minimum of all possible scales)
	float max_scale = MIN(MIN(1 / rgb_ratio[0], 1 / rgb_ratio[1]), 1 / rgb_ratio[2]);
	float scale = MIN(Y / total_Y, max_scale);

	for (int i = 0; i < 3; i++) {
		rgb_ratio[i] = clamp(scale * rgb_ratio[i], 0.0f, 1.0f);
	}

	rgb[RED] = (uint16_t)(rgb_ratio[0] * 0xffff);
	rgb[GREEN] = (uint16_t)(rgb_ratio[1] * 0xffff);
	rgb[BLUE] = (uint16_t)(rgb_ratio[2] * 0xffff);
}
