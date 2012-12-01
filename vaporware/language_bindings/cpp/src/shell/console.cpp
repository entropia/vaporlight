#include "console.hpp"
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



#include <cstring>
#include <sstream>
#include <stdexcept>

#include <readline/readline.h>
#include <readline/history.h>

void init_readln() {
	using_history();
}

bool readln(std::string& line,const std::string& prompt) {
	static char * last_cmd = nullptr;
	char * tmp = readline(prompt.c_str());
	if (!tmp) {
		return false;
	}
	
	line = tmp;
	
	if (*tmp && last_cmd) {
		//don't safe duplicates:
		if (strcmp(tmp, last_cmd)) {
			add_history(tmp);
		}
	}
	else
		if (!last_cmd) {
			add_history(tmp);
		}
	if (last_cmd) {
		free(last_cmd);
	}
	last_cmd = tmp;
	
	return true;
}


std::pair< std::string, std::vector< std::string > > parse_cmd(
    const std::string& cmd,
    const std::map<std::string, std::string>& argmap
) {
	using std::string;
	std::istringstream stream(cmd);
	string primary_command;
	if (!stream.good()) {
		throw std::invalid_argument("empty commandline cannot be parsed");
	}
	stream >> primary_command;
	auto tmp_it = argmap.find(primary_command);
	if (tmp_it != argmap.end()) {
		primary_command = tmp_it->second;
	}
	
	std::vector<std::string> args;
	std::string tmp;
	while (stream.good()) {
		stream >> tmp;
		args.push_back(tmp);
	}
	
	return std::make_pair(primary_command, args);
}
