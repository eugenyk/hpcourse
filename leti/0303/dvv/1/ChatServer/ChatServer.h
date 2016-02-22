#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <deque>
#include <atomic>
#include <boost/thread/condition.hpp>
#include "ChatMessage.h"

using boost::asio::ip::tcp;

class chat_participant
{
public:
	virtual ~chat_participant() {};
	virtual void deliver(const chat_message& msg) = 0;
};

class chat_room
{
private:
	std::set<std::shared_ptr<chat_participant>> participants_;
	enum { max_recent_msgs = 100 };
	std::deque<chat_message> recent_msgs_;
	boost::shared_mutex mutex_;
	boost::shared_mutex mutexPart_;
	std::atomic<long long> messCount = 0;
public:

	void join(std::shared_ptr<chat_participant> participant);
	void leave(std::shared_ptr<chat_participant> participant);
	void addRecentMsg(const chat_message& msg);
	void deliver(const chat_message& msg);

private:
	void addPart(std::shared_ptr<chat_participant> participant);
};

class chat_server
{
private:
	std::shared_ptr<boost::thread> thread_;
	boost::asio::io_service& io_service_;
	std::deque<chat_message> systemMessQueue_;
	boost::mutex mutex_;
	boost::condition cond;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	chat_room room_;
public:
	chat_server(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint)
		: io_service_(io_service),
		acceptor_(io_service, endpoint),
		socket_(io_service)
	{
		thread_ = std::make_shared<boost::thread>(boost::bind(&chat_server::handleMessage, this));
		do_accept();
	}

	void addCmd(const chat_message& mess);

	bool isStopped() const { return io_service_.stopped(); }
private:
	void do_accept();
	void handleMessage();
};

class chat_session
	: public chat_participant,
	public std::enable_shared_from_this<chat_session>
{
private:
	tcp::socket socket_;
	chat_server& chat_;
	chat_room& room_;
	chat_message read_msg_;
public:
	chat_session(tcp::socket socket, chat_room& room, chat_server& server)
		: socket_(std::move(socket)),
		room_(room),
		chat_(server)
	{
	}

	void start();
	void deliver(const chat_message& msg);

private:
	void do_read_header();
	void do_read_body();
	void do_write(const chat_message& msg);
};

#endif