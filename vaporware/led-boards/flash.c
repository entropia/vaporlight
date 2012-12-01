#include "flash.h"

#include "stm_include/stm32/flash.h"

#include "config.h"
#include "error.h"

#ifdef TRACE_FLASH
#include "debug.h"
#include "term.h"
#endif

/*
 * Unlocks the flash memory for programming.
 */
error_t flash_unlock() {
	if (FLASH_CR & FLASH_LOCK) {
		FLASH_KEYR = FLASH_KEY1;
		FLASH_KEYR = FLASH_KEY2;

		// Check if unlock was successful.
		if (FLASH_CR & FLASH_LOCK) {
			error(ER_BUG, STR_WITH_LEN("Flash unlocking failed"), EA_PANIC);
		}
	}
	
	return E_SUCCESS;
}

/*
 * Locks the flash memory. Programming is no longer possible
 */
error_t flash_lock() {
	FLASH_CR |= FLASH_LOCK;
	return E_SUCCESS;
}

/*
 * Checks the flash status for errors.
 */
static error_t flash_check_error() {
	// Wait for write to finish
	// Need to wait one more cycle, see erratum 2.7
	__asm("nop");
	while (FLASH_SR & FLASH_BSY);

	if (FLASH_SR & (FLASH_PGERR | FLASH_WRPRTERR)) {
#ifdef TRACE_FLASH
		debug_string("Flash error, FLASH_SR = ");
		debug_hex(FLASH_SR, 8);
		debug_string(CRLF);
#endif
		return E_FLASH_WRITE;
	} else {
		return E_SUCCESS;
	}
}

/*
 * Writes a word to flash and checks it afterwards.
 */
error_t flash_write_check(uint16_t *address, uint16_t value) {
	error_t error;

	// Enable programming
	FLASH_CR |= FLASH_PG;
	
	*address = value;
		
	error = flash_check_error();

	FLASH_CR &= ~FLASH_PG;
	
	if (error != E_SUCCESS) return error;

	if (*address != value) return E_FLASH_WRITE;

	return E_SUCCESS;
}

/*
 * Erases a page in flash. base_addr must point to the beginning of
 * the page.
 */
error_t flash_erase_page(void *base_addr) {
#ifdef TRACE_FLASH
	debug_string("Erasing flash page at ");
	debug_hex(base_addr, 8);
	debug_string(CRLF);
#endif

	while(FLASH_SR & FLASH_BSY);
	
	if (FLASH_CR & FLASH_LOCK)
		error(ER_BUG, STR_WITH_LEN("Flash is locked"), EA_PANIC);
	
	FLASH_CR |= FLASH_PER;
	FLASH_AR = (uint32_t) base_addr;
	FLASH_CR |= FLASH_STRT;
	
	error_t error = flash_check_error();

	FLASH_CR &= ~FLASH_PER;
	
	if (error) return error;

	// Verify
	uint16_t *start = (uint16_t*) base_addr;
	for (uint16_t *i = start;
	     i < start + FLASH_PAGE_SIZE/sizeof(uint16_t);
	     i++) {
		if (*i != 0xffff) {
#ifdef TRACE_FLASH
			debug_string("Verification failed at ");
			debug_hex(i, 8);
			debug_string(" with ");
			debug_hex(*i, 8);
			debug_string(CRLF);
#endif
			return E_FLASH_WRITE;
		}
	}
	
	return E_SUCCESS;
}

/*
 * Copies data from source (anywhere in the address space) to
 * destination (must be in flash and on an erased page). Data is
 * copied in halfwords, of which hw_count are copied.
 *
 * When an error occurs, the function aborts copying and returns
 * E_FLASH_WRITE. Otherwise, it returns E_SUCCESS.
 */
error_t flash_copy(void *destination, void *source, int hw_count) {
	error_t error;
	
	uint16_t *dest = (uint16_t*) destination;
	uint16_t *src = (uint16_t*) source;

	for (int i = 0; i < hw_count; i++) {
		error = flash_write_check(dest + i, src[i]);
		if (error != E_SUCCESS) return error;
	}

	return E_SUCCESS;
}
