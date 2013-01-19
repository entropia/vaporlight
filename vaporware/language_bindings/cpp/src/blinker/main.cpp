/*
 *  This file is part of vaporpp.
 *
 *  vaporpp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vaporpp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vaporpp.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "../lib/client.hpp"
#include "../util/ids.hpp"
#include "../util/colors.hpp"



void control_LEDs(std::vector<uint16_t> LEDs);

void fade_to(const std::vector<uint16_t>& LEDs, 
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color);

void set_leds(std::vector<uint16_t> LEDs, const vlpp::rgba_color& col);

struct settings{
	static int fade_steps;
	static useconds_t min_sleep_time;
	static useconds_t max_sleep_time;
	static useconds_t min_fade_time;
	static useconds_t max_fade_time;
	static std::vector<vlpp::rgba_color> colorset;
	static vlpp::client client;
};
int settings::fade_steps = UINT8_MAX;
useconds_t settings::min_sleep_time = 0;
useconds_t settings::max_sleep_time = 1000;
useconds_t settings::min_fade_time  = 0;
useconds_t settings::max_fade_time  = 1000;
std::vector<vlpp::rgba_color> settings::colorset = REAL_COLORS;
vlpp::client settings::client;

/*
 * this program will make all lights blink
 */
int main ( int argc, char**argv ) {
	using std::string;
	
	string server;
	string token;
	uint16_t port;
	std::string LED_string;
	std::vector<uint16_t> LEDs;
	bool async = true;
	std::string colorset_str;
	
	// this try-block may not be the best style,
	// but it is required to enforce stack-unwinding 
	// in error-cases:
	try {
		boost::program_options::options_description desc;
		using boost::program_options::value;
		desc.add_options()
			("help,h", "print this help")
			("sync,y", "makes the LEDs blink asynchronus.")
			("token,t", value<std::string>(&token), "sets the authentication-token")
			("server,s", value<std::string>(&server), "sets the servername")
			("port,p", value<uint16_t>(&port)->default_value ( vlpp::client::DEFAULT_PORT ),
				"sets the server-port")
			("leds,l", value<std::string>(&LED_string), "sets the number of leds")
			("min-sleep", value<useconds_t>(&settings::min_sleep_time), 
				"changes the minimum sleep-time")
			("max-sleep,S", value<useconds_t>(&settings::max_sleep_time), 
				"changes the maximum sleep-time")
			("colors,c", value<std::string>(&colorset_str), "sets the used colorset")
			("min-fade", value<useconds_t>(&settings::min_fade_time), "changes the minimum fade time")
			("max-fade,f", value<useconds_t>(&settings::max_fade_time), "changes the maximum fade time")
			("fade-steps,F", value<int>(&settings::fade_steps), "sets the number of steps for fading");
		
		boost::program_options::variables_map vm;
		boost::program_options::store (boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify ( vm );
		
		if(vm.count("help")){
			std::cout << desc << std::endl;
			return 0;
		}
		
		vm.count("sync") && (async = false);
		settings::colorset = str_to_cols(colorset_str);
		LEDs = str_to_ids(LED_string);
		
		settings::client = vlpp::client(server, token, port);
		if(async){
			std::vector<std::thread> threads;
			for(auto LED: LEDs){
				threads.emplace_back(control_LEDs, std::vector<uint16_t> {LED});
			}
			for(auto& thread: threads){
				thread.join();
			}
		} else {
			control_LEDs(LEDs);
		}
	} catch(std::exception& e){
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
}


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
			uint8_t(old_color.r * p_old + new_color.r * p_new),
			uint8_t(old_color.g * p_old + new_color.g * p_new),
			uint8_t(old_color.b * p_old + new_color.b * p_new),
			uint8_t(old_color.alpha * p_old + new_color.alpha * p_new)
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
