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


#ifndef IDS_HPP
#define IDS_HPP

#include <vector>
#include <cstdint>
#include <string>

/**
 * @brief converts a string to a list of uint16_t
 * @param str the string
 * @return a vector of the numbers
 */
std::vector<uint16_t> str_to_ids(const std::string& str);

#endif // IDS_HPP
