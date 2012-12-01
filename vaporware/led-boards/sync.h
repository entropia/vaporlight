/*
 * Increments the value pointed to by ptr atomically.
 */
#define atomic_increment(ptr) __sync_fetch_and_add(ptr, 1)

/*
 * Decrements the value pointed to by ptr atomically.
 */
#define atomic_decrement(ptr) __sync_fetch_and_sub(ptr, 1)

/*
 * Sets the value pointed to by ptr to 1.
 * The value must have been 1 or 0 before.
 */
#define atomic_set(ptr) __sync_fetch_and_or(ptr, 1)

/*
 * Sets the value pointed to by ptr to 0.
 * The value must have been 1 or 0 before.
 */
#define atomic_reset(ptr) __sync_fetch_and_and(ptr, 0)

/*
 * Suspends interrupts.
 */
#define interrupts_off() __asm("cpsid i" : : : "memory")

/*
 * Resume interrupts.
 */
#define interrupts_on() __asm("cpsie i" : : : "memory")
