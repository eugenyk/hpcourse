#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "ChatMessage.h"
#include "ChatClient.h"

using boost::asio::ip::tcp;

void chat_client::write(const chat_message& msg)
{
	io_service_.post(
		[this, msg]()
	{
		do_write(msg);
	});
}

void chat_client::close()
{
	io_service_.post([this]() { socket_.close(); });
}

void chat_client::do_connect(tcp::resolver::iterator endpoint_iterator)
{
	boost::asio::async_connect(socket_, endpoint_iterator,
		[this](boost::system::error_code ec, tcp::resolver::iterator it)
	{
		if (!ec) do_read_header();
	});
}

void chat_client::do_read_header()
{
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), chat_message::header_length),
		[this](boost::system::error_code ec, std::size_t)
	{
		if (!ec && read_msg_.decode_header()) do_read_body();
		else socket_.close();
	});
}

void chat_client::do_read_body()
{
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
		[this](boost::system::error_code ec, std::size_t)
	{
		if (!ec)
		{
			if (read_msg_.decode_body())
			{
				std::cout << read_msg_().sender() << ": ";
				std::cout << read_msg_().text() << "\n";
				read_msg_().Clear();
			}
			else std::cout << "\n!Decoding error!\n";
			do_read_header();
		}
		else socket_.close();
	});
}

void chat_client::do_write(const chat_message& msg)
{
	std::shared_ptr<std::string> s = std::make_shared<std::string>(msg.data(), msg.length());
	boost::asio::async_write(socket_,
		boost::asio::buffer(s->c_str(),
			s->length()),
		[this, s](boost::system::error_code ec, std::size_t)
	{
		if (ec)	socket_.close();
	});
}
