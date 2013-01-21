#include "core.hpp"

#include <cstdint>
#include <random>
#include <chrono>
#include <mutex>

#include "settings.hpp"



void control_LEDs(std::vector<uint16_t> LEDs) {
	// first set up the random-number-generators:
	std::default_random_engine generator(
		static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()) );
	std::uniform_int_distribution<useconds_t> sleep_time_distribution(
			settings::min_sleep_time, settings::max_sleep_time);
	std::uniform_int_distribution<useconds_t> fade_time_distribution(
			settings::min_fade_time, settings::max_fade_time);
	std::uniform_int_distribution<size_t> color_distribution(0, settings::colorset.size() - 1);
	
	// and now start the actual work:
	vlpp::rgba_color last_color;
	while(true){
		vlpp::rgba_color tmp = settings::colorset[color_distribution(generator)];
		if(tmp == last_color){
			continue;
		}
		fade_to(LEDs, fade_time_distribution(generator), last_color, tmp);
		last_color = tmp;
		usleep (sleep_time_distribution(generator));
	}
}

void fade_to(const std::vector<uint16_t>& LEDs,
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color){
	useconds_t time_per_step = fade_time / settings::fade_steps;
	for(int i=0; i < settings::fade_steps; ++i){
		double p_new = double(i) / settings::fade_steps;
		double p_old = 1 - p_new;
		vlpp::rgba_color tmp{
			// i really WANT this narrowing conversion:
			uint8_t(old_color.r*p_old + new_color.r*p_new),
			uint8_t(old_color.g*p_old + new_color.g*p_new),
			uint8_t(old_color.b*p_old + new_color.b*p_new),
			uint8_t(old_color.alpha*p_old + new_color.alpha*p_new)
		};
		set_leds(LEDs, tmp);
		usleep(time_per_step);
	}
	set_leds(LEDs, new_color);
}


void set_leds(std::vector<uint16_t> LEDs, const vlpp::rgba_color& col){
	static std::mutex m;
	std::lock_guard<std::mutex> lock(m);
	for(auto LED: LEDs){
		settings::client.set_led(LED, col);
	}
	settings::client.flush();
}
