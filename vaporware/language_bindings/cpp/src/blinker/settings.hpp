#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <unistd.h>

#include "../lib/client.hpp"
#include "../util/colors.hpp"

struct settings{
	static int fade_steps;
	static useconds_t min_sleep_time;
	static useconds_t max_sleep_time;
	static useconds_t min_fade_time;
	static useconds_t max_fade_time;
	static std::vector<vlpp::rgba_color> colorset;
	static vlpp::client client;
};

#endif
