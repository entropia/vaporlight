#include "color.h"

#include "console.h"
#include "debug.h"
#include "fixedpoint.h"
#include "term.h"

static void mat_x_vec(fixed_t m[static 9], fixed_t x[static 3], fixed_t result[static 3]) {
	result[0] = fixadd3(fixmul(m[0], x[0]), fixmul(m[1], x[1]), fixmul(m[2], x[2]));
	result[1] = fixadd3(fixmul(m[3], x[0]), fixmul(m[4], x[1]), fixmul(m[5], x[2]));
	result[2] = fixadd3(fixmul(m[6], x[0]), fixmul(m[7], x[1]), fixmul(m[8], x[2]));
}

static fixed_t dot(fixed_t a[static 3], fixed_t b[static 3]) {
	return fixadd3(fixmul(a[0], b[0]),
		       fixmul(a[1], b[1]),
		       fixmul(a[2], b[2]));
}

static fixed_t clamp(fixed_t x, fixed_t min, fixed_t max) {
	if (fixlt(x, min)) {
		return min;
	} else if (fixgt(x, max)) {
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
void color_correct(led_info_t info,
		   uint16_t x, uint16_t y, uint16_t Y,
		   uint16_t rgb[static 3]) {

	// First, get the ratio of the PWM channels right.  This is
	// done by finding the barycentric coordinates of xyY within
	// the LED's gamut.
	// info already contains the inverted matrix
	// necessary to compute this.

	fixed_t in[3] = {
		fixfract(x),
		fixfract(y),
		FIXNUM(1.0)
	};
	fixed_t rgb_ratio[3];

	mat_x_vec(info.color_matrix, in, rgb_ratio);

	// Approximate the desired color with one that is actually
	// in the gamut (i.e. 0 <= rgb_ratio[i] <= 1).
	// TODO This does not actually find the closest match.
	for (int i = 0; i < 3; i++) {
		rgb_ratio[i] = clamp(rgb_ratio[i], FIXNUM(0.0), FIXNUM(1.0));
	}

	// Now adjust for the desired luminosity.
	// Note that the color takes precedence: If reproducing the
	// color at the requested luminosity it not possible, it will
	// be reproduced darker.
	fixed_t total_Y = dot(rgb_ratio, info.peak_Y);

	fixed_t scale = fixdiv(fixnum(Y), total_Y);

	for (int i = 0; i < 2; i++) {
		if (fixne(rgb_ratio[i], FIXNUM(0.0))) {
			scale = fixmin(scale,
				       fixdiv(FIXNUM(1.0), rgb_ratio[i]));
		}
	}

	for (int i = 0; i < 3; i++) {
		rgb_ratio[i] = fixmul(rgb_ratio[i], scale);
		rgb[i] = fix_fract_part(rgb_ratio[i]);
	}
}

/*
 * Converts a flatly counted channel index (i.e. 0 to 15 standing for
 * LED0 red, LED0 green, LED0 blue, LED1 red, ...) to the PWM channel
 * index to use for this LED.
 */
uint8_t convert_channel_index(uint8_t c) {
	if (c == 15) {
		return config.backup_channel;
	} else {
		uint8_t led = c / 3;
		uint8_t channel = c % 3;
		return config.led_infos[led].channels[channel];
	}
}

/*
 * Writes the inverse of the 3x3 matrix pointed to by in to the 3x3
 * matrix pointed to by out.
 */
void invert_3x3(fixed_t in[static 9], fixed_t out[static 9]) {
	fixed_t invdet =
		fixdiv(FIXNUM(1.0),
		       fixadd3(
			              fixmul(in[0], fixsub(fixmul(in[8],in[4]), fixmul(in[7],in[5]))),
			       fixneg(fixmul(in[3], fixsub(fixmul(in[8],in[1]), fixmul(in[7],in[2])))),
			              fixmul(in[6], fixsub(fixmul(in[5],in[1]), fixmul(in[4],in[2])))));

	out[0] = fixmul(invdet,        fixsub(fixmul(in[8],in[4]), fixmul(in[7],in[5])));
	out[1] = fixmul(invdet, fixneg(fixsub(fixmul(in[8],in[1]), fixmul(in[7],in[2]))));
	out[2] = fixmul(invdet,        fixsub(fixmul(in[5],in[1]), fixmul(in[4],in[2])));
	out[3] = fixmul(invdet, fixneg(fixsub(fixmul(in[8],in[3]), fixmul(in[6],in[5]))));
	out[4] = fixmul(invdet,        fixsub(fixmul(in[8],in[0]), fixmul(in[6],in[2])));
	out[5] = fixmul(invdet, fixneg(fixsub(fixmul(in[5],in[0]), fixmul(in[3],in[2]))));
	out[6] = fixmul(invdet,        fixsub(fixmul(in[7],in[3]), fixmul(in[6],in[4])));
	out[7] = fixmul(invdet, fixneg(fixsub(fixmul(in[7],in[0]), fixmul(in[6],in[1]))));
	out[8] = fixmul(invdet,        fixsub(fixmul(in[4],in[0]), fixmul(in[3],in[1])));
}
