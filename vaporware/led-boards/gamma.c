#include "gamma.h"

#include <string.h>

#include "flash.h"

#include "gamma_table.inc"

/*
 * Copy of the gamma tables in RAM for faster access and editing
 * capability.
 */
static uint16_t ram_gamma_table[4][256];

/*
 * Effectively computes ((raw_brightness / 255) ^ gamma(color)) * (1 << PWM_BITS).
 *
 * This is done by accessing the precomputed gamma table.
 */
uint16_t gamma(color_t color, uint8_t raw_brightness) {
	int color_code = (int) color;
	return ram_gamma_table[color_code][raw_brightness];
}

/*
 * Initializes the gamma module by loading the gamma table into
 * RAM. This must be called before any other function in this module.
 */
void gamma_init() {
	memcpy(ram_gamma_table, gamma_table, sizeof(ram_gamma_table));
}

/*
 * Edits the gamma tables in RAM, setting
 * gamma(color, index) = gamma_value.
 */
void gamma_edit(color_t color, uint8_t index, uint16_t gamma_value) {
	ram_gamma_table[color][index] = gamma_value;
}

/*
 * Reloads the gamma tables from flash.
 */
error_t gamma_reload() {
	memcpy(ram_gamma_table, gamma_table, sizeof(ram_gamma_table));
	return E_SUCCESS;
}

/*
 * Saves the current gamma tables to flash.
 */
error_t gamma_save() {
	error_t error;
	
	_Static_assert(sizeof(gamma_table) == sizeof(ram_gamma_table),
		       "Gamma tables have different sizes?!");
	_Static_assert(sizeof(gamma_table) % FLASH_PAGE_SIZE == 0,
		       "Gamma table must fill pages completely");

	flash_unlock();

	int pagecount = sizeof(gamma_table) / FLASH_PAGE_SIZE;
	for(int p = 0; p < pagecount; p++) {
		uint8_t *base_addr = ((uint8_t*) gamma_table) + p * FLASH_PAGE_SIZE;
		error = flash_erase_page(base_addr);
		if (error != E_SUCCESS) goto out;
	}
	
	error = flash_copy(&gamma_table, &ram_gamma_table,
		   sizeof(gamma_table) / sizeof(uint16_t));
	if (error != E_SUCCESS) goto out;

out:
	flash_lock();
	return error;
}
