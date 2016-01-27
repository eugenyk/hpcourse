//
#include "inc_server.h"


using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;
typedef boost::condition Condition;
typedef boost::shared_mutex SharedMutex;
typedef boost::lock_guard<SharedMutex> WriteLock;
typedef boost::shared_lock<SharedMutex> ReadLock;
typedef boost::condition Condition;
typedef boost::mutex Mutex;
typedef boost::unique_lock<Mutex> Lock;
typedef boost::shared_ptr<chat_participant> chat_participant_ptr;

void chat_room::join(chat_participant_ptr participant)
{
	WriteLock lockPart(mutex_participants_);
	participants_.insert(participant);
	ReadLock lockMsgs(mutex_msgs_);
	std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
		boost::bind(&chat_participant::deliver, participant, _1));
}

void chat_room::leave(chat_participant_ptr participant)
{
	WriteLock lock(mutex_participants_);
	participants_.erase(participant);
}

void chat_room::deliver(const chat_message& msg)
{
	WriteLock lock(mutex_msgs_);
	recent_msgs_.push_back(msg);
	while (recent_msgs_.size() > max_recent_msgs)
		recent_msgs_.pop_front();

	std::for_each(participants_.begin(), participants_.end(),
		boost::bind(&chat_participant::deliver, _1, boost::ref(msg)));
}

chat_session::chat_session(boost::asio::io_service& io_service, chat_room& room, chat_server& server)
	: socket_(io_service),
	room_(room),
	server_(server)
{
}

tcp::socket& chat_session::socket()
{
	return socket_;
}

void chat_session::start()
{
	room_.join(shared_from_this());
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.mdata(), chat_message::header_length),
		boost::bind(
			&chat_session::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
}

void chat_session::deliver(const chat_message& msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().mdata(),
				write_msgs_.front().length()),
			boost::bind(&chat_session::handle_write, shared_from_this(),
				boost::asio::placeholders::error));
	}
}

void chat_session::handle_read_header(const boost::system::error_code& error)
{
	if (!error && read_msg_.decode_header())
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			boost::asio::transfer_all(),
			boost::bind(&chat_session::handle_read_body, shared_from_this(),
				boost::asio::placeholders::error));
	}
	else
	{
		room_.leave(shared_from_this());
	}
}

void chat_session::handle_read_body(const boost::system::error_code& error)
{
	if (!error)
	{
		read_msg_.decode();
		if (read_msg_.text().compare(0,3,"/c ") == 0)
		{
			server_.addCmd(read_msg_);
		}
		else
		{
			room_.deliver(read_msg_);
		}
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.mdata(), chat_message::header_length),
			boost::bind(&chat_session::handle_read_header, shared_from_this(),
				boost::asio::placeholders::error));
	}
	else
	{
		room_.leave(shared_from_this());
	}
}

void chat_session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		write_msgs_.pop_front();
		if (!write_msgs_.empty())
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				boost::bind(&chat_session::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
		}
	}
	else
	{
		room_.leave(shared_from_this());
	}
}

typedef boost::shared_ptr<chat_session> chat_session_ptr;

chat_server::chat_server(boost::asio::io_service& io_service,
	const tcp::endpoint& endpoint)
	: io_service_(io_service),
	acceptor_(io_service, endpoint)
{
	thread_ = std::make_shared<boost::thread>(boost::bind(&chat_server::handle_message, this));
	start_accept();
}

void chat_server::addCmd(const chat_message& mess) 
{
	boost::unique_lock<boost::mutex> lock(mutex_cmd_);
	cmdMessQueue_.push_back(mess);
	cond.notify_one();
}

void chat_server::start_accept()
{
	chat_session_ptr new_session(new chat_session(io_service_, room_,*this));
	acceptor_.async_accept(new_session->socket(),
		boost::bind(&chat_server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
}

void chat_server::handle_accept(chat_session_ptr session,
	const boost::system::error_code& error)
{
	if (!error)
	{
		session->start();
	}

	start_accept();
}
	
void chat_server::handle_message() {
	while (!io_service_.stopped()) {
		std::string cmdWithArg;
		boost::unique_lock<boost::mutex> lock(mutex_cmd_);
		while (cmdMessQueue_.empty()) {
			cond.wait(lock);
		}
		chat_message m(cmdMessQueue_.front());
		m.decode();
		cmdWithArg = m.text();
		cmdMessQueue_.pop_front();
			
		cmdWithArg.erase(0,3);
		std::string cmd;
		std::string args;
		std::stringstream ss;
		ss<<cmdWithArg;
		if (ss >> cmd)
		{
			std::cout << m.sender() << " " << cmd << " ";
			std::string arg;
			if (cmd == "print") {
				
				while (ss >> arg)
				{
					std::cout << arg << " ";
				}
			}
			else if (cmd == "exit") {
				io_service_.stop();
			}
			std::cout << std::endl;
		}
	}
}

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cerr << "Usage: chat_server <threads_count> <port> [<port> ...]\n";
			return 1;
		}

		boost::asio::io_service io_service;

		chat_server_list servers;
		for (int i = 2; i < argc; ++i)
		{
			using namespace std; // For atoi.
			tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
			chat_server_ptr server(new chat_server(io_service, endpoint));
			servers.push_back(server);
		}
		boost::thread_group threads;
		int threads_count = std::min<int>(atoi(argv[1]),1);
		for (int i = 0; i < threads_count; i += 1)
		{
			threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
		}
		threads.join_all();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
