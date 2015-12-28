#ifndef SERVER_H_
#define SERVER_H_

#include <thread>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include "message.h"

namespace network {

class server {
	std::vector<std::thread> threads;
	unsigned thread_count;
public:
	class tcp;
protected:
	boost::asio::io_service service;
	short unsigned port;
public:
	server(unsigned thread_count, short unsigned port): thread_count(thread_count), port(port) {
		threads.reserve(thread_count);
	}
	~server() { stop(); }
protected:
	void run();
	void stop();
};

class server::tcp: public server
{
	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::acceptor acceptor;
public:
	class connection
	{
		friend server::tcp;
	private:
		class implementation;
		std::shared_ptr<implementation> impl;
	protected:
		connection(const std::shared_ptr<implementation>& impl): impl(impl) {}
		connection(const connection& other): impl(other.impl) {}
	public:
		connection(connection&& other): impl(std::move(other.impl)) {}
		connection& operator=(connection&& other) {
			impl = std::move(other.impl);
			return *this;
		}
		std::string getAddress() const;
	public:
		bool operator==(const connection& other) const;
		bool operator!=(const connection& other) const { return !(*this == other); }
		void receive(message&& message);
		void send(const message& message);
	public:
		~connection();
	};
public:
	tcp(unsigned thread_count, short unsigned port);
	~tcp();

public:
	std::function<void(connection&&)> onConnect    = [](connection&&) {};
	std::function<void(connection&)>  onDisconnect = [](connection&)  {};
	std::function<void(connection&, message)> onSend    = [](connection&, message) {};
	std::function<void(connection&, message)> onReceive = [](connection&, message) {};

	void transmit(connection& client, const message& message)  { client.send(message); }
private:
	void doConnect();
};

} /* namespace network */

#endif /* SERVER_H_ */
