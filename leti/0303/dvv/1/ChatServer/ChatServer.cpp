#include <deque>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include "ChatMessage.h"
#include "ChatServer.h"

using boost::asio::ip::tcp;

void chat_room::join(std::shared_ptr<chat_participant> participant)
{
	addPart(participant);
	boost::shared_lock<boost::shared_mutex> lock(mutex_);
	for (auto msg : recent_msgs_)
		participant->deliver(msg);
}

void chat_room::leave(std::shared_ptr<chat_participant> participant)
{
	boost::lock_guard<boost::shared_mutex> lock(mutexPart_);
	participants_.erase(participant);
}

void chat_room::addRecentMsg(const chat_message& msg) 
{
	boost::lock_guard<boost::shared_mutex> lock(mutex_);
	recent_msgs_.push_back(msg);
	while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();
}

void chat_room::deliver(const chat_message& msg)
{
	addRecentMsg(msg);
	for (auto participant : participants_)
		participant->deliver(msg);
}

void chat_room::addPart(std::shared_ptr<chat_participant> participant) 
{
	boost::lock_guard<boost::shared_mutex> lock(mutexPart_);
	participants_.insert(participant);
}

void chat_server::addCmd(const chat_message& mess) 
{
	boost::unique_lock<boost::mutex> lock(mutex_);
	systemMessQueue_.push_back(mess);
	cond.notify_one();
}

void chat_server::handleMessage()
{
	const std::string::size_type start = 3;
	while (!io_service_.stopped()) 
	{
		std::string cmdWithArg;
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			while (systemMessQueue_.empty()) cond.wait(lock);
			chat_message m(systemMessQueue_.front());
			cmdWithArg = m().text().substr(start);
			systemMessQueue_.pop_front();
		}

		std::string::size_type pos = cmdWithArg.find(' ');
		std::string cmd;
		std::string args;
		if (pos != std::string::npos) 
		{
			cmd = cmdWithArg.substr(0, pos);
			args = cmdWithArg.substr(pos + 1);
		}
		else cmd = cmdWithArg;
		
		if (cmd == "stopServer") io_service_.stop();
	}
}

void chat_session::start()
{
	room_.join(shared_from_this());
	do_read_header();
}

void chat_session::deliver(const chat_message& msg)
{
	do_write(msg);
}


void chat_session::do_read_header()
{
	if (!chat_.isStopped()) {
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			[this, self](boost::system::error_code ec, std::size_t)
		{
			if (!ec) if (read_msg_.decode_header()) do_read_body();
			else room_.leave(shared_from_this());
		});
	}
}

void chat_session::do_read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.body(), read_msg_.body_length()), boost::asio::transfer_all(),
		[this, self](boost::system::error_code ec, std::size_t)
	{
		if (!ec)
		{
			if (read_msg_.decode_body()) 
			{
				const std::string& s = read_msg_().text();
				//std::cout << "Message:" << s << std::endl;
				std::string findStr("/c");
				if (s.size() > findStr.size() && s.substr(0, findStr.size()) == findStr) chat_.addCmd(read_msg_);
				else room_.deliver(read_msg_);
				do_read_header();
			}
		else room_.leave(shared_from_this());
		}
	});
}

void chat_session::do_write(const chat_message& msg)
{
	std::shared_ptr<std::string> s = std::make_shared<std::string>(msg.data(), msg.length());
	auto self(shared_from_this());
	boost::asio::async_write(socket_,
		boost::asio::buffer(s->data(),
			s->length()),
		[this, self, s](boost::system::error_code ec, std::size_t)
	{
		if (ec) room_.leave(shared_from_this());
	});
}

void chat_server::do_accept()
{
	acceptor_.async_accept(socket_,
		[this](boost::system::error_code ec)
	{
		if (!ec) std::make_shared<chat_session>(std::move(socket_), room_, *this)->start();
		do_accept();
	});
}
