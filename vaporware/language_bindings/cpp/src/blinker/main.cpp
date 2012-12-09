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
#include <map>
#include <cmath>
#include <cctype>
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



/**
 * @brief control_single_LED
 * @param cl
 * @param m
 * @param LED
 */
void control_LEDs(vlpp::client& cl, std::mutex& m, std::vector<uint16_t> LEDs, useconds_t max_sleep_time, const std::vector<vlpp::rgba_color> &colors);


/*
 * this program will torture anyone in the room
 */
int main(int argc, char**argv) {
	using std::string;
	namespace bpo = boost::program_options;
	
	string server;
	string token;
	uint16_t port;
	std::string LED_string;
	std::vector<uint16_t> LEDs;
	useconds_t max_sleep_time;
	bool async = false;
	std::string colorset_str;
	
	try{
		bpo::options_description desc;
		desc.add_options()
				("help,h", "print this help")
				("token,t", bpo::value<std::string>(&token), "sets the authentication-token")
				("server,s", bpo::value<std::string>(&server), "sets the servername")
				("port, p", bpo::value<uint16_t>(&port)->default_value(vlpp::client::DEFAULT_PORT),
				 "sets the server-port")
				("leds,l", bpo::value<std::string>(&LED_string), "sets the number of leds")
				("max-sleep,S", bpo::value<useconds_t>(&max_sleep_time)->default_value(100000),
				 "changes the maximum sleep-time")
				("async,a", "makes the LEDs blink asynchronus.")
				("colors,c", bpo::value<std::string>(&colorset_str), "sets the used colorset");
		
		bpo::variables_map vm;
		bpo::store(bpo::parse_command_line(argc, argv, desc) ,vm);
		bpo::notify(vm);
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}
		if (vm.count("async")) {
			async =  true;
		}
		auto colorset = BLACK_WHITE;
		if(colorset_str == "all"){
			colorset = ALL_COLORS;
		}
		else if(colorset_str == "real"){
			colorset == REAL_COLORS;
		}
		else if(colorset_str == "most"){
			colorset = MOST_COLORS;
		}
		
		LEDs = str_to_ids(LED_string);
		if (LEDs.empty()){
			std::cerr << "Error: You need to provide the "
				"IDs of at least one LED." << std::endl;
			return 1;
		}
		
		vlpp::client client(server, token, port);
		std::mutex m;
		if(async){
			// this is, where the actual programm starts:
			std::vector<std::thread> threads;
			for(auto LED: LEDs){
				threads.push_back(std::thread(control_LEDs, std::ref(client), 
					std::ref(m), std::vector<uint16_t>{LED}, 
					max_sleep_time, colorset ));
			}
			for(auto& t: threads){
				t.join();
			}
		}
		else{
			control_LEDs(client, std::ref(m), LEDs, max_sleep_time, colorset);
		}
		
	}
	catch(std::exception& e){
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
}


void control_LEDs(vlpp::client& cl, std::mutex& m, 
		std::vector<uint16_t> LEDs, useconds_t max_sleep_time, 
		const std::vector<vlpp::rgba_color>& colors){
	std::default_random_engine generator(
		(unsigned long)std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<useconds_t> time_distribution(1,max_sleep_time);
	std::uniform_int_distribution<size_t> color_distribution(0, colors.size() - 1);
	vlpp::rgba_color last_color;
	while(true){
		vlpp::rgba_color tmp = colors[color_distribution(generator)];
		if(tmp != last_color) {
			std::lock_guard<std::mutex> lock(m);
			for(auto LED: LEDs){
				cl.set_led(LED, tmp);
			}
			cl.flush();
		}
		else{
			continue;
		}
		last_color = tmp;
		usleep(time_distribution(generator));
	}
}
