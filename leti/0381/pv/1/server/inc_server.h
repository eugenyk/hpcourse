#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <sstream>

#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>
#include <boost\asio.hpp>
#include <boost\thread.hpp>
#include <boost\thread\mutex.hpp>
#include <boost\thread\condition.hpp>

#include "chat_message.h"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

typedef boost::shared_mutex SharedMutex;
typedef boost::lock_guard<SharedMutex> WriteLock;
typedef boost::shared_lock<SharedMutex> ReadLock;
typedef boost::condition Condition;
typedef boost::mutex Mutex;
typedef boost::unique_lock<Mutex> Lock;

//----------------------------------------------------------------------

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const chat_message& msg) = 0;
};

typedef boost::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
	void join(chat_participant_ptr participant);

	void leave(chat_participant_ptr participant);

	void deliver(const chat_message& msg);

private:
	std::set<chat_participant_ptr> participants_;
	enum { max_recent_msgs = 100 };
	chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------
class chat_server;

class chat_session
	: public chat_participant,
	public boost::enable_shared_from_this<chat_session>
{
public:
	chat_session(boost::asio::io_service& io_service, chat_room& room, chat_server& server);

	tcp::socket& socket();

	void start();

	void deliver(const chat_message& msg);

	void handle_read_header(const boost::system::error_code& error);

	void handle_read_body(const boost::system::error_code& error);

	void handle_write(const boost::system::error_code& error);

private:
	tcp::socket socket_;
	chat_room& room_;
	chat_server& server_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------

class chat_server
{
public:
	chat_server(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint);

	void addCmd(const chat_message& mess);

	void start_accept();

	void handle_accept(chat_session_ptr session,
		const boost::system::error_code& error);
	
	void handle_message();

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	chat_room room_;

	std::shared_ptr<boost::thread> thread_;
	chat_message_queue cmdMessQueue_;
	boost::mutex mutex_;
	boost::condition cond;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------
