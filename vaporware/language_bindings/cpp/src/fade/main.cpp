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

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "../lib/client.hpp"
#include "../util/ids.hpp"

#include "color_calculation.hpp"


/*
 * this program will just fade through most colors
 */
int main(int argc, char**argv) {
	using std::string;
	namespace bpo = boost::program_options;
	
	string server;
	string token;
	uint16_t port;
	std::string LED_string;
	std::vector<uint16_t> LEDs;
	uint8_t alpha;
	double timestep;
	
	try{
		bpo::options_description desc;
		desc.add_options()
				("help,h", "print this help")
				//("verbose,v", "be verbose")
				("token,t", bpo::value<std::string>(&token), "sets the authentication-token")
				("server,s", bpo::value<std::string>(&server), "sets the servername")
				("port, p", bpo::value<uint16_t>(&port)->default_value(vlpp::client::DEFAULT_PORT),
				 "sets the server-port")
				("leds,l", bpo::value<std::string>(&LED_string), "sets the number of leds")
				("alpha,a", bpo::value<uint8_t>(&alpha)->default_value(UINT8_MAX), 
				 "sets the alpha-channel")
				("timestep,T", bpo::value<double>(&timestep)->default_value(0.1),
				 "sets the time between lightchanges");
		
		bpo::variables_map vm;
		bpo::store(bpo::parse_command_line(argc, argv, desc) ,vm);
		bpo::notify(vm);
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}
		
		LEDs = str_to_ids(LED_string);
		if (LEDs.empty()){
			std::cerr << "Error: You need to provide the "
				"IDs of at least one LED." << std::endl;
			return 1;
		}
		
		vlpp::client client(server, token, port);
		
		uint16_t color_degree_counter = 0;
		double color_degree;
		while(true){
			color_degree_counter += UINT8_MAX/4;
			color_degree = (double)color_degree_counter / UINT16_MAX;
			vlpp::rgba_color tmp = calc_deg_color(color_degree);
			//std::cout << tmp << std::endl;
			tmp.alpha = alpha;
			for(auto id: LEDs){ 
				client.set_led(id,tmp);
			}
			client.flush();
			usleep( (useconds_t)(1000000*timestep) );
		}
	}
	catch(std::exception& e){
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
}

