#ifndef CORE_HPP
#define CORE_HPP

#include "../util/colors.hpp"

#include <cstdint>
#include <vector>

#include <unistd.h>

/**
 * @brief Takes control of some LEDs and let's them fade in some random ways.
 * @param LEDs the IDs of the LEDs that will be controlled by this function
 */
void control_LEDs(std::vector<uint16_t> LEDs);

/**
 * @brief fades some LEDs to a new color.
 * @param LEDs a vector that contains the LED-IDs
 * @param fade_time the time that will be used to fade to the new color.
 * @param old_color the old color
 * @param new_color the new color
 */
void fade_to(const std::vector<uint16_t>& LEDs, 
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color);

/**
 * @brief Sets some LEDs to a new color in a synchronized way.
 * @param LEDs a vector that contains the LED-IDs
 * @param col the new color
 */
void set_leds(std::vector<uint16_t> LEDs, const vlpp::rgba_color& col);




#endif
