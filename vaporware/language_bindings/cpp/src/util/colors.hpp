#ifndef COLORS_HPP
#define COLORS_HPP
#include <vector>
#include <map>
#include <string>

#include "../lib/rgba_color.hpp"

const uint8_t MAX_CHANNEL_BRIGHTNESS = UINT8_MAX;

const vlpp::rgba_color WHITE(MAX_CHANNEL_BRIGHTNESS, MAX_CHANNEL_BRIGHTNESS, 
	MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);
const vlpp::rgba_color BLACK(0, 0, 0, UINT8_MAX);

const vlpp::rgba_color RED(MAX_CHANNEL_BRIGHTNESS, 0, 0, UINT8_MAX);
const vlpp::rgba_color BLUE(0, 0, MAX_CHANNEL_BRIGHTNESS, UINT8_MAX);
const vlpp::rgba_color GREEN(0, MAX_CHANNEL_BRIGHTNESS, 0, UINT8_MAX);
const vlpp::rgba_color YELLOW(MAX_CHANNEL_BRIGHTNESS, MAX_CHANNEL_BRIGHTNESS, 0, UINT8_MAX);


const std::vector<vlpp::rgba_color> BLACK_WHITE = {BLACK, WHITE};
const std::vector<vlpp::rgba_color> REAL_COLORS = {RED, BLUE, GREEN, YELLOW};
const std::vector<vlpp::rgba_color> ALL_COLORS  = {BLACK, WHITE, RED, BLUE, GREEN, YELLOW};
const std::vector<vlpp::rgba_color> MOST_COLORS = {WHITE, RED, BLUE, GREEN, YELLOW};

const std::map<std::string, const vlpp::rgba_color> COLOR_MAP = {
	{"black", BLACK}, {"white", WHITE},
	{"red", RED}, {"blue", BLUE}, {"green", GREEN}, {"yellow", YELLOW}
};

#endif // COLORS_HPP
