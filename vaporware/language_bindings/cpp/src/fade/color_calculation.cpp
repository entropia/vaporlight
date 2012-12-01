#include "color_calculation.hpp"

#include <cmath>

constexpr double SIN_FACTOR = 2 * M_PI;
constexpr double G_CHANNEL_SHIFT =  SIN_FACTOR / 3;
constexpr double B_CHANNEL_SHIFT = 2 * SIN_FACTOR / 3;

vlpp::rgba_color calc_deg_color(double degree){
	vlpp::rgba_color returncolor;
	returncolor.r = uint8_t( UINT8_MAX * (sin(SIN_FACTOR * degree) + 1)/2 );
	returncolor.g = uint8_t( UINT8_MAX * (sin(SIN_FACTOR * degree + G_CHANNEL_SHIFT ) + 1)/2 );
	returncolor.b = uint8_t( UINT8_MAX * (sin(SIN_FACTOR * degree + B_CHANNEL_SHIFT ) + 1)/2 );
	return returncolor;
}
