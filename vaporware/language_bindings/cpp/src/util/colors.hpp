#ifndef COLORS_HPP
#define COLORS_HPP
#include <vector>
#include <map>
#include <string>

#include "../lib/rgba_color.hpp"

/**
 * @brief Converts a string to a list of colors.
 * @param str the string
 * @return a vector that contains the colors
 */
std::vector<vlpp::rgba_color> str_to_cols(const std::string& str);

/**
 * @brief converts a string to a color.
 * @param str the string
 * @return the color that was represented by the string
 */
vlpp::rgba_color str_to_col(const std::string& str);

const uint8_t MAX_CHANNEL_BRIGHTNESS = UINT8_MAX;

const vlpp::rgba_color WHITE(MAX_CHANNEL_BRIGHTNESS, MAX_CHANNEL_BRIGHTNESS, 
	MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);
const vlpp::rgba_color BLACK(0, 0, 0, UINT8_MAX);

const vlpp::rgba_color RED(MAX_CHANNEL_BRIGHTNESS, 0, 0, UINT8_MAX);
const vlpp::rgba_color BLUE(0, 0, MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);
const vlpp::rgba_color GREEN(0, MAX_CHANNEL_BRIGHTNESS, 0, UINT8_MAX);
const vlpp::rgba_color YELLOW(MAX_CHANNEL_BRIGHTNESS, MAX_CHANNEL_BRIGHTNESS, 0, UINT8_MAX);
const vlpp::rgba_color CYAN(0, MAX_CHANNEL_BRIGHTNESS, MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);
const vlpp::rgba_color MAGENTA(MAX_CHANNEL_BRIGHTNESS, 0, MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);


const std::vector<vlpp::rgba_color> BLACK_WHITE = {BLACK, WHITE};
const std::vector<vlpp::rgba_color> REAL_COLORS = {RED, BLUE, GREEN, YELLOW, CYAN, 
	MAGENTA};
const std::vector<vlpp::rgba_color> ALL_COLORS  = {BLACK, WHITE, RED, BLUE, GREEN, 
	YELLOW, CYAN, MAGENTA};
const std::vector<vlpp::rgba_color> MOST_COLORS = {WHITE, RED, BLUE, GREEN, YELLOW,
	CYAN, MAGENTA};

const std::map<std::string, std::vector<vlpp::rgba_color>> COLOR_SETS_MAP = {
	{"b_w", BLACK_WHITE}, {"real", REAL_COLORS}, {"all", ALL_COLORS},
	{"most", MOST_COLORS}
};

const std::map<std::string, const vlpp::rgba_color> COLOR_MAP = {
	{"black", BLACK}, {"white", WHITE},
	{"red", RED}, {"blue", BLUE}, {"green", GREEN}, {"yellow", YELLOW},
	{"cyan", CYAN}, {"magenta", MAGENTA}
};

#endif // COLORS_HPP
