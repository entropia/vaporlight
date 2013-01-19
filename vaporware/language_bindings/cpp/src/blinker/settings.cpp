#include "settings.hpp"

int settings::fade_steps = UINT8_MAX;
useconds_t settings::min_sleep_time = 0;
useconds_t settings::max_sleep_time = 100000;
useconds_t settings::min_fade_time  = 0;
useconds_t settings::max_fade_time  = 100000;
std::vector<vlpp::rgba_color> settings::colorset = REAL_COLORS;
vlpp::client settings::client;
