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


#include "ids.hpp"

#include <cctype>
#include <stdexcept>

std::vector<uint16_t> str_to_ids(const std::string& str) {
	using std::vector;
	using std::string;
	
	vector<uint16_t> returnlist;
	string current_word;
	uint16_t tmp_id = 0;
	bool in_range = false;
	for (auto c: str) {
		if (isdigit(c)) {
			current_word += c;
		}
		else
			if (c==',') {
				if (in_range) {
					uint16_t upper_bound = (uint16_t)std::stoul(current_word);
					if (upper_bound < tmp_id) {
						throw std::invalid_argument("invalid range");
					}
					for (uint16_t i = tmp_id; i <= upper_bound; ++i) {
						returnlist.push_back(i);
					}
				}
				else {
					returnlist.push_back((uint16_t)std::stoul(current_word));
				}
				current_word.clear();
			}
			else
				if (c == '-') {
					if (in_range) {
						throw std::invalid_argument("invalid range");
					}
					in_range = true;
					tmp_id = (uint16_t)std::stoul(current_word);
					current_word.clear();
				}
				else {
					throw std::invalid_argument("invalid range");
				}
	}
	if (!current_word.empty()) {
		if (in_range) {
			uint16_t upper_bound = (uint16_t)std::stoul(current_word);
			if (upper_bound < tmp_id) {
				throw std::invalid_argument("invalid range");
			}
			for (uint16_t i = tmp_id; i <= upper_bound; ++i) {
				returnlist.push_back(i);
			}
		}
		else {
			returnlist.push_back((uint16_t)std::stoul(current_word));
		}
	}
	return returnlist;
}
