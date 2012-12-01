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


#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>

#include "../lib/client.hpp"

/**
 * @brief Adds commands to set the given leds to the given color to the buffer. 
 * @note This wont flush the buffer!
 * @param cl A refercence to the conncection-client
 * @param leds a string that contains describes the leds to be set
 * @param color a string that describes a color
 */
void set_leds(vlpp::client& cl, const std::string& leds, const std::string& color);


/**
 * @brief Prints a help-message for the commandline.
 */
void print_cli_help();

#endif // COMMANDS_HPP
