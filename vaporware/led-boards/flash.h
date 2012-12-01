#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#include "error.h"

// Size of a flash page
#define FLASH_PAGE_SIZE 1024

/*
 * Unlocks the flash memory for programming.
 */
error_t flash_unlock();

/*
 * Locks the flash memory. Programming is no longer possible
 */
error_t flash_lock();

/*
 * Writes a word to flash and checks it afterwards.
 */
error_t flash_write_check(uint16_t *address, uint16_t value);

/*
 * Erases a page in flash. base_addr must point to the beginning of
 * the page.
 */
error_t flash_erase_page(void *base_addr);

/*
 * Copies data from source (anywhere in the address space) to
 * destination (must be in flash and on an erased page). Data is
 * copied in halfwords, of which hw_count are copied.
 *
 * When an error occurs, the function aborts copying and returns
 * E_FLASH_WRITE. Otherwise, it returns E_SUCCESS.
 */
error_t flash_copy(void *destination, void *source, int hw_count);

#endif
