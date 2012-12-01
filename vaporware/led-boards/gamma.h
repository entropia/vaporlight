#ifndef GAMMA_H
#define GAMMA_H

#include <stdint.h>

#include "error.h"
#include "led.h"

/*
 * Effectively computes ((raw_brightness / 255) ^ gamma(color)) * (1 << PWM_BITS).
 *
 * This is done by accessing the precomputed gamma table.
 */
uint16_t gamma(color_t color, uint8_t raw_brightness);

/*
 * Initializes the gamma module by loading the gamma table into
 * RAM. This must be called before any other function in this module.
 */
void gamma_init();

/*
 * Edits the gamma tables in RAM, setting
 * gamma(color, index) = gamma_value.
 */
void gamma_edit(color_t color, uint8_t index, uint16_t gamma_value);

/*
 * Reloads the gamma tables from flash.
 */
error_t gamma_reload();

/*
 * Saves the current gamma tables to flash.
 */
error_t gamma_save();

#endif
