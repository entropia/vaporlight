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


#include "client.hpp"

#include <cassert>

#include <boost/asio.hpp>
#include <array>

using boost::asio::io_service;
using boost::asio::ip::tcp;


//pimpl-class (private members of client):
class vlpp::client::client_impl {
	public:
		client_impl(const std::string& servername, const std::string& token, uint16_t port);
		void authenticate(const std::string& token);
		void set_led(uint16_t led, rgba_color col);
		void flush();
		io_service _io_service;
		tcp::socket _socket;
		std::vector<char> cmd_buffer;
};


//opcodes:
enum:
uint8_t {
	OP_SET_LED = 0x01,
	OP_AUTHENTICATE = 0x02,
	OP_STROBE = 0xFF
};

enum { TOKEN_SIZE = 16 };

///////////


vlpp::client::client(const std::string &server, const std::string &token, uint16_t port):
	_impl(new vlpp::client::client_impl(server, token, port)) {
}

vlpp::client::~client() {
}

void vlpp::client::set_led(uint16_t led_id, const rgba_color &col) {
	_impl->set_led(led_id, col);
}

void vlpp::client::set_leds(const std::vector<uint16_t> &led_ids, const rgba_color &col) {
	for (auto led: led_ids) {
		set_led(led, col);
	}
}

void vlpp::client::flush() {
	_impl->flush();
}


///////// now: the private stuff


vlpp::client::client_impl::client_impl(const std::string &servername, const std::string &token, uint16_t port):
	_socket(_io_service) {
	//first check the token:
	if (token.length() != TOKEN_SIZE) {
		throw std::invalid_argument("invalid token (wrong size)");
	}
	
	tcp::resolver _resolver(_io_service);
	tcp::resolver::query q(servername, std::to_string(port));
	auto endpoints = _resolver.resolve(q);
	boost::asio::connect(_socket, endpoints);
	if (!_socket.is_open()) {
		throw std::runtime_error("cannot open socket");
	}
	authenticate(token);
}

void vlpp::client::client_impl::authenticate(const std::string &token) {
	assert(token.length() == TOKEN_SIZE);
	std::array<char,TOKEN_SIZE+1> auth_data;
	auth_data[0] = OP_AUTHENTICATE;
	for (size_t i = 0; i < TOKEN_SIZE; ++i) {
		auth_data[i+1] = (char)token[i];
	}
	boost::system::error_code e;
	boost::asio::write(_socket, boost::asio::buffer(&(auth_data[0]), auth_data.size()) , e);
	if (e) {
		throw std::runtime_error("write failed");
	}
}

void vlpp::client::client_impl::set_led(uint16_t led, rgba_color col) {
	cmd_buffer.push_back((char)OP_SET_LED);
	cmd_buffer.push_back((char)(led >> 8));
	cmd_buffer.push_back((char)(led & 0xff));
	cmd_buffer.push_back((char)col.r);
	cmd_buffer.push_back((char)col.g);
	cmd_buffer.push_back((char)col.b);
	cmd_buffer.push_back((char)col.alpha);
}

void vlpp::client::client_impl::flush() {
	cmd_buffer.push_back((char)OP_STROBE);
	boost::system::error_code e;
	boost::asio::write(_socket, boost::asio::buffer(&(cmd_buffer[0]), cmd_buffer.size()), e);
	cmd_buffer.clear();
	if (e) {
		throw std::runtime_error("write failed");
	}
}
