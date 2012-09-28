#ifndef HEAT_H
#define HEAT_H

/*
 * The type for overheat handler functions.
 */
typedef void (*heat_handler_t)();

/*
 * Initializes temperature sensors. When this function has been called,
 * a heat check will be done at each system timer tick. When overheat is
 * detected, the function on_overheat is called.
 */
void heat_init(heat_handler_t on_overheat);

/*
 * This function does a heat check on every system timer tick.
 */
void heat_timer_tick();


#endif
