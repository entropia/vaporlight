#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * val is packed in a struct to prevent inadvertant usage of + - * /.
 */
typedef struct {
	int32_t v;
} fixed_t;

#define FRAC_BITS 16

/*
 * Statically convert a float/double literal to fixed_t.
 *
 * This should only be used with constant argument to avoid including
 * the soft FPU.
 *
 * FIXINIT is suitable for initializers, FIXNUM for constants.
 */
#define FIXINIT(f) {((((int)(f)) << 16) + (int)((f - (int)(f)) * (1 << FRAC_BITS)))}
#define FIXNUM(f) ((fixed_t) FIXINIT(f))

/*
 * Returns a fixed point number with the value n.
 */
fixed_t fixnum(int16_t n);

/*
 * Returns a fixed point number with the value n/65536.
 */
fixed_t fixfract(uint16_t n);

/*
 * Returns the integral part of f.
 */
int16_t fix_int_part(fixed_t f);

/*
 * Returns the fractional part of f.
 */
uint16_t fix_fract_part(fixed_t f);

/*
 * Basic arithmetic operations on fixed point numbers.
 */
fixed_t fixadd(fixed_t f, fixed_t g);
fixed_t fixsub(fixed_t f, fixed_t g);
fixed_t fixmul(fixed_t f, fixed_t g);
fixed_t fixdiv(fixed_t f, fixed_t g);

/*
 * Unary negation
 */
fixed_t fixneg(fixed_t f);

/*
 * Convenience functions for longer sums and products.
 */
fixed_t fixadd3(fixed_t f, fixed_t g, fixed_t h);

/*
 * Relational operators.
 */
bool fixlt(fixed_t f, fixed_t g);
bool fixgt(fixed_t f, fixed_t g);
bool fixle(fixed_t f, fixed_t g);
bool fixge(fixed_t f, fixed_t g);
bool fixeq(fixed_t f, fixed_t g);
bool fixne(fixed_t f, fixed_t g);

/*
 * Return minimum and maximum.
 */
fixed_t fixmin(fixed_t f, fixed_t g);
fixed_t fixmax(fixed_t f, fixed_t g);

#endif
