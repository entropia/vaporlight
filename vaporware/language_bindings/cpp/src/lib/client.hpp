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


#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include "rgba_color.hpp"

namespace vlpp {


/**
 * @brief The client class, used to connect to the server.
 *
 * This class provides a low-level-interface used to communicate with the server.
 *
 * Note that using this class is NOT threadsafe.
 */
class client {
	public:
	
		/**
		 * @brief the default port
		 */
		enum: uint16_t { DEFAULT_PORT = 7534 };
		
		/**
		 * @brief the default constructor.
		 * 
		 * Note that this is not properly constructed afterwards, so any
		 * attempt of using it will result in a vlpp::uninitialized_error 
		 * beeing thrown.
		 *
		 */
		client() = default;
		
		/**
		 * @brief Constructs an instance, connects to the specified server and authenticates there.
		 * @param server the servername; this might be an ip-address or an hostname,
		 *               eg "192.168.23.44" or "example.com"
		 * @param token the authentication-token
		 * @param port the server-port
		 * @throws std::invalid_argument if the token has an invalid size
		 * @throws vlpp::connection_failure if no connection could be created or a write fails
		 */
		client(const std::string& server, const std::string& token, uint16_t port = DEFAULT_PORT);
		
		/**
		 * @brief move-ctor
		 * @param other an rvalue-reference to another instance
		 */
		client(client&& other);
		
		/**
		 * @brief Asigns an rvalue-instance to this.
		 * @param the rvalue-instance
		 * @return a reference to *this
		 */
		client& operator=(client&& other);
		
		/**
		 * @brief cleans up the object.
		 */
		~client();
		
		/**
		 * @brief authenticate at the server
		 * @param token the authentication-token
		 * @throws std::invalid_argument if the token has an invalid size
		 * @throws vlpp::uninitialized_error if this is not initialized correctly
		 */
		void authenticate(const std::string& token);
		
		/**
		 * @brief Sets a rgb-LED to a specific rgba-color.
		 * @param led_id the ID of the led
		 * @param col the new color of the LED
		 * @throws vlpp::uninitialized_error if this is not initialized correctly
		 */
		void set_led(uint16_t led_id, const rgba_color& col);
		
		/**
		 * @brief Sets a list of LEDs to a specific color.
		 * @param led_ids the IDs of the LEDs
		 * @param col the new color of the LEDs
		 * @throws vlpp::uninitialized_error if this is not initialized correctly
		 */
		void set_leds(const std::vector<uint16_t>& led_ids, const rgba_color& col);
		
		/**
		 * @brief execute the sent commands
		 * @throws std::runtime_error if the write fails
		 * @throws vlpp::uninitialized_error if this is not initialized correctly
		 */
		void flush();
		
		
	private:
		// we are using the pimpl-idiom to decrease the
		// compiletime and dependencies for users of this class:
		class client_impl;
		std::unique_ptr<client_impl> _impl;
};

/**
 * @brief Exception that will be thrown if the connection fails
 */
class connection_failure : public std::runtime_error {
	public:
		/**
		 * @brief The usual exception ctor.
		 * @param msg the error-message
		 */
		connection_failure(const std::string& msg) : std::runtime_error(msg){}
};

/**
 * @brief Exception to be thrown if an object is used uninitialized
 */
class uninitialized_error : public std::runtime_error {
	public:
		/**
		 * @brief The usual exception ctor.
		 * @param msg the error-message
		 */
		uninitialized_error(const std::string& msg) : std::runtime_error(msg){}
};

}//namespace vlpp


#endif // CLIENT_HPP
