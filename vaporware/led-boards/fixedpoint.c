#include "fixedpoint.h"

#define MKFIX(x) (fixed_t) { x };

fixed_t fixnum(int16_t n) {
	return MKFIX(n << FRAC_BITS);
}

fixed_t fixfract(uint16_t n) {
	return MKFIX(n);
}

int16_t fix_int_part(fixed_t f) {
	return (int16_t)(f.v >> FRAC_BITS);
}

uint16_t fix_fract_part(fixed_t f) {
	return (uint16_t)(f.v);
}

fixed_t fixadd(fixed_t f, fixed_t g) {
	return MKFIX(f.v + g.v);
}

fixed_t fixadd3(fixed_t f, fixed_t g, fixed_t h) {
	return MKFIX(f.v + g.v + h.v);
}

fixed_t fixsub(fixed_t f, fixed_t g) {
	return MKFIX(f.v - g.v);
}

fixed_t fixneg(fixed_t f) {
	return MKFIX(-f.v);
}

fixed_t fixmul(fixed_t f, fixed_t g) {
	int64_t f64 = (int64_t) f.v;
	int64_t g64 = (int64_t) g.v;
	return MKFIX((f64 * g64) >> FRAC_BITS);
}

fixed_t fixdiv(fixed_t f, fixed_t g) {
	int64_t f64 = (int64_t) f.v;
	return MKFIX((f64 << FRAC_BITS) / g.v);
}

bool fixlt(fixed_t f, fixed_t g) {
	return f.v < g.v;
}

bool fixgt(fixed_t f, fixed_t g) {
	return f.v > g.v;
}

bool fixle(fixed_t f, fixed_t g) {
	return f.v <= g.v;
}

bool fixge(fixed_t f, fixed_t g) {
	return f.v >= g.v;
}

bool fixeq(fixed_t f, fixed_t g) {
	return f.v == g.v;
}

bool fixne(fixed_t f, fixed_t g) {
	return f.v != g.v;
}

fixed_t fixmin(fixed_t f, fixed_t g) {
	if (fixlt(f, g)) {
		return f;
	} else {
		return g;
	}
}

fixed_t fixmax(fixed_t f, fixed_t g) {
	if (fixgt(f, g)) {
		return f;
	} else {
		return g;
	}
}
