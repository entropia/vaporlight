#include "fail.h"

/*
 * A simple module to manage failures.
 */

/*
 * Counts the number of set bits in the argument.
 */
static int popcount(int x) {
	int bits;

	for (bits = 0; x != 0; bits++) {
		x &= x - 1;
	}

	return bits;
}

/*
 * Initializes the given failure record.
 */
void fail_init(fail_t *record, int treshold) {
	record->failure_record = 0;
	record->treshold = treshold;
}

/*
 * Records one new event in the given record.
 *
 * Returns 1, if too many failures were recorded lately.
 */
uint8_t fail_event(fail_t *record, int is_failure) {
	// Forget one event
	record->failure_record = record->failure_record << 1;

	if (is_failure) record->failure_record++;

	return popcount(record->failure_record) >= record->treshold;
}
