#ifndef FAIL_H
#define FAIL_H

#include <stdint.h>

/*
 * A simple module to manage failures.
 */

/*
 * Everything that is needed to keep track of one
 * point of failure.
 */
typedef struct {
	// A record of the last eight events.
	// Failure is recorded as 1, success as 0.
	int failure_record;
	
	// The treshold for failure counts.
	// If more events than this value are failures,
	// the system reports an error.
	int treshold;
} fail_t;

/*
 * Initializes the given failure record.
 */
void fail_init(fail_t *record, int treshold);

/*
 * Records one new event in the given record.
 *
 * Returns 1, if too many failures were recorded lately.
 */
uint8_t fail_event(fail_t *record, int is_failure);

#endif
