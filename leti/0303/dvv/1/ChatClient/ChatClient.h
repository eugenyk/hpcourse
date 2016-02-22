#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <mutex>
#include "ChatMessage.h"

using boost::asio::ip::tcp;

class chat_client
{
private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	chat_message read_msg_;
	std::string name_;
	std::mutex mutex_;

public:
	chat_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator,
		const std::string& name)
		: io_service_(io_service),
		socket_(io_service),
		name_(name)
	{
		do_connect(endpoint_iterator);
	}

	void write(const chat_message& msg);
	void close();

private:
	void do_connect(tcp::resolver::iterator endpoint_iterator);
	void do_read_header();
	void do_read_body();
	void do_write(const chat_message& msg);
};

#endif