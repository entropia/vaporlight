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
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "../lib/client.hpp"
#include "../util/ids.hpp"
#include "../util/colors.hpp"

#include "settings.hpp"
#include "core.hpp"


/*
 * this program will make all lights blink
 */
int main(int argc, char**argv){
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

