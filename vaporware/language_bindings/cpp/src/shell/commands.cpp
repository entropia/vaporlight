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

#include <iostream>

#include "commands.hpp"

#include "../lib/rgba_color.hpp"

#include "ids.hpp"

void set_leds(vlpp::client& cl, const std::string& leds, const std::string& color) {
	vlpp::rgba_color col(color);
	cl.set_leds(str_to_ids(leds), col);
}

void print_cli_help(){
	std::cout << "Commands: \n\n"
		     "set|s <LEDs> <rgba-colorcode>\n"
		     "\tsets the leds to a specific color\n"
		     "add|a <LEDs> <rgba-colorcode>\n"
		     "\tbuffers commands to set some leds to a color\n"
		     "flush|f\n"
		     "\texecutes the buffered commands\n"
		     "quit|q\n"
		     "\tquit the programm\n"
		     "help|h\n"
		     "\tprint this help\n"
		<< std::endl;
}
