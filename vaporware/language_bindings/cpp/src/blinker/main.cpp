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
#include <cstdio>
#include <stdexcept>
#include <map>
#include <cmath>
#include <cctype>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "../lib/client.hpp"
#include "../util/ids.hpp"
#include "../util/colors.hpp"



void control_LEDs(vlpp::client& cl, std::mutex& m, std::vector<uint16_t> LEDs);

void fade_to(vlpp::client& cl, std::mutex& m, const std::vector<uint16_t>& LEDs, 
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color);

struct settings{
	static int fade_steps;
	static useconds_t max_sleep_time;
	static useconds_t max_fade_time;
	static std::vector<vlpp::rgba_color> colorset;
};
int settings::fade_steps = UINT8_MAX;
useconds_t settings::max_sleep_time = 1000;
useconds_t settings::max_fade_time  = 1000;
std::vector<vlpp::rgba_color> settings::colorset = BLACK_WHITE;

/*
 * this program will make all lights blink
 */
int main ( int argc, char**argv ) {
	using std::string;
	
	using boost::program_options::store;
	using boost::program_options::notify;
	using boost::program_options::parse_command_line;
	using boost::program_options::options_description;
	using boost::program_options::value;
	using boost::program_options::variables_map;
	
	string server;
	string token;
	uint16_t port;
	std::string LED_string;
	std::vector<uint16_t> LEDs;
	bool async = false;
	std::string colorset_str;
	
	try {
		options_description desc;
		desc.add_options()
			( "help,h", "print this help" )
			( "token,t", value<std::string>(&token), "sets the authentication-token" )
			( "server,s", value<std::string>(&server), "sets the servername" )
			( "port, p", value<uint16_t>(&port)->default_value ( vlpp::client::DEFAULT_PORT ),
				"sets the server-port" )
			( "leds,l", value<std::string>(&LED_string), "sets the number of leds" )
			( "max-sleep,S", value<useconds_t>(&settings::max_sleep_time), 
				"changes the maximum sleep-time" )
			( "async,a", "makes the LEDs blink asynchronus." )
			( "colors,c", value<std::string>(&colorset_str), "sets the used colorset" )
			( "max-fade,f", value<useconds_t>(&settings::max_fade_time), "changes the maximum fade time")
			( "fade-steps,F", value<int>(&settings::fade_steps), "sets the number of steps for fading");
		
		variables_map vm;
		store ( parse_command_line ( argc, argv, desc ) ,vm );
		notify ( vm );
		if ( vm.count ( "help" ) ) {
			std::cout << desc << std::endl;
			return 0;
		}
		if ( vm.count ( "async" ) ) {
			async =  true;
		}
		if ( colorset_str == "all" ) {
			settings::colorset = ALL_COLORS;
		} else if ( colorset_str == "real" ) {
			settings::colorset = REAL_COLORS;
		} else if ( colorset_str == "most" ) {
			settings::colorset = MOST_COLORS;
		}
		
		LEDs = str_to_ids ( LED_string );
		if ( LEDs.empty() ) {
			std::cerr << "Error: You need to provide the "
				  "IDs of at least one LED." << std::endl;
			return 1;
		}
		
		vlpp::client client ( server, token, port );
		std::mutex m;
		if ( async ) {
			std::vector<std::thread> threads;
		for ( auto LED: LEDs ) {
				threads.push_back ( std::thread (
					control_LEDs, std::ref ( client ),
					std::ref ( m ), std::vector<uint16_t> {LED}) );
			}
		for ( auto& t: threads ) {
				t.join();
			}
		} else {
			control_LEDs ( client, std::ref ( m ), LEDs);
		}
		
	} catch ( std::exception& e ) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
}


void control_LEDs ( vlpp::client& cl, std::mutex& m, std::vector<uint16_t> LEDs) {
	std::default_random_engine generator (
		static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()) );
	std::uniform_int_distribution<useconds_t> sleep_time_distribution ( 0,settings::max_sleep_time );
	std::uniform_int_distribution<useconds_t> fade_time_distribution ( 0,settings::max_fade_time );
	std::uniform_int_distribution<size_t> color_distribution ( 0, settings::colorset.size() - 1 );
	
	vlpp::rgba_color last_color;
	while ( true ) {
		vlpp::rgba_color tmp = settings::colorset[color_distribution ( generator )];
		if ( tmp == last_color ) {
			continue;
		}
		fade_to(cl, m, LEDs, fade_time_distribution(generator), last_color, tmp);
		last_color = tmp;
		usleep (sleep_time_distribution(generator));
	}
}

void fade_to( vlpp::client& cl, std::mutex& m, const std::vector<uint16_t>& LEDs, 
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color){
	useconds_t time_per_step = fade_time / settings::fade_steps;
	for(int i=0; i < settings::fade_steps; ++i){
		double p_new = double(i) / settings::fade_steps;
		double p_old = 1 - p_new;
		vlpp::rgba_color tmp{
			// i really WANT this conversion to uint8_t:
			uint8_t(old_color.r * p_old + new_color.r * p_new),
			uint8_t(old_color.g * p_old + new_color.g * p_new),
			uint8_t(old_color.b * p_old + new_color.b * p_new),
			uint8_t(old_color.alpha * p_old + new_color.alpha * p_new)
		};
		{
			std::lock_guard<std::mutex> lock(m);
			for( auto LED: LEDs ){
				cl.set_led(LED, tmp);
			}
			cl.flush();
		}
		usleep(time_per_step);
	}
	{
		std::lock_guard<std::mutex> lock(m);
		for(auto LED: LEDs){
			cl.set_led(LED, new_color);
		}
		cl.flush();
	}
}
