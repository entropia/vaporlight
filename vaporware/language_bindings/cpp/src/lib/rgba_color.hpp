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

#ifndef RGBA_COLOR_HPP
#define RGBA_COLOR_HPP

#include <cstdint>
#include <string>
#include <ostream>

namespace vlpp {

/**
 * @brief The rgba_color class
 */
class rgba_color {
	public:
		/**
		 * @brief default-ctor; will initialize #000000ff
		 */
		rgba_color() = default;
		
		/**
		 * @brief the default copy-ctor
		 * @param other an already existing instance that will be copied
		 */
		rgba_color(const rgba_color& other) = default;
		
		/**
		 * @brief the default assignement-function
		 * @param other an already existing instance that will be copied
		 */
		rgba_color& operator=(const rgba_color& other) = default;
		
		/**
		 * @brief Constructs an rgba-color from the provided arguments
		 * @param r the red-value
		 * @param g the green-value
		 * @param b the blue-value
		 * @param alpha the alpha-value
		 */
		rgba_color(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = UINT8_MAX);
		
		/**
		 * @brief Constructs a color from a string.
		 * @param colorcode the color as a string (like #ffffff or #ffffffff)
		 * @throws std::invalid_argument if the string cannot be converted to a color
		 */
		rgba_color(std::string colorcode);
		
		/**
		 * @brief Compares two colors.
		 * @param other the other color
		 * @return true if the colors are identical, false otherwise
		 */
		bool operator==(const rgba_color& other) const;
		
		/**
		 * @brief Compares two colors.
		 * @param other the other color
		 * @return false if the colors are identical, true otherwise
		 */
		bool operator!=(const rgba_color& other) const;
		
		/**
		 * @brief Compares two colors and provides an implementation-defined ordering.
		 *
		 * This is primarily to enable usage in sorted containers like std::set.
		 * 
		 * @param other the other color.
		 * @return true if this is ordered in front of the other color.
		 */
		bool operator<(const rgba_color& other) const;
		
		/**
		 * @brief the red-value
		 */
		uint8_t r = 0;
		
		/**
		 * @brief the green-value
		 */
		uint8_t g = 0;
		
		/**
		 * @brief the blue-value
		 */
		uint8_t b = 0;
		
		/**
		 * @brief the alpha-value
		 */
		uint8_t alpha = UINT8_MAX;
};

}

/**
 * @brief Writes an rgba-color to a stream
 * @param stream the stream
 * @param col the color
 * @return the original stream
 */
std::ostream& operator<<(std::ostream& stream, const vlpp::rgba_color& col);


#endif // RGBA_COLOR_HPP
