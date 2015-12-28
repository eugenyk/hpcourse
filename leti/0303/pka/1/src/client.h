// Project: Chat
//
//  Created on: 14 нояб. 2015 г.
//      Author: motorhead
//

#ifndef CLIENT_H_
#define CLIENT_H_

#include <thread>
#include <mutex>
#include <functional>
#include <boost/asio.hpp>
#include "message.h"

namespace network {

class client {
	boost::asio::io_service service;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::endpoint endpoint;
	std::mutex send_lock, receive_lock;
	std::thread process;
public:

	client(std::string ip, short unsigned port);
	~client();
	void send(const network::message message);
	void receive(network::message message);
public:
	std::function<void(network::message)> onSend = [](network::message) {};
	std::function<void(network::message)> onReceive = [](const network::message) {};
};

} /* namespace network */

#endif /* CLIENT_H_ */
