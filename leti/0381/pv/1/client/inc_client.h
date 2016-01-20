#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <sstream>

#include "chat_message.h"
typedef std::deque<chat_message> chat_message_queue;

using boost::asio::ip::tcp;

class chat_client
{
public:
	chat_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator);

	void write(const chat_message& msg);

	void close();

private:

	void handle_connect(const boost::system::error_code& error);

	void handle_read_header(const boost::system::error_code& error);

	void handle_read_body(const boost::system::error_code& error);

	void do_write(chat_message msg);

	void handle_write(const boost::system::error_code& error);

	void do_close();

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};
