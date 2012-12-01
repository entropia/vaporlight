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


#ifndef READLN_HPP
#define READLN_HPP

#include <string>
#include <utility>
#include <vector>
#include <map>


/**
 * @brief initialize readline. Call this once before you use readln
 */
void init_readln();

/**
 * @brief c++-wrapper for GNU-readline
 * @param line a reference to string that will be used to save the read line
 * @param prompt the commandprompt
 * @return false if EOF is reached, true otherwise
 */
bool readln(std::string& line, const std::string& prompt);

/**
 * @brief parse a command
 * @param cmd the command from the commandline
 * @return a pair of the primary command and a list of the arguments
 */
std::pair< std::string, std::vector< std::string > > parse_cmd(const std::string& cmd,
	const std::map<std::string, std::string>& argmap = {{}});

#endif // READLN_HPP
