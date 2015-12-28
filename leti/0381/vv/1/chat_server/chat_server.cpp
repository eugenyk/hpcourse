#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "chat_message.h"
#include <mutex>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

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

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:

  void join(chat_participant_ptr participant)
  {
	  addPart(participant);
	  ReadLock lock(mutex_);
    for (auto msg: recent_msgs_)
      participant->deliver(msg);
  }

  void leave(chat_participant_ptr participant)
  {
	  WriteLock lock(mutexPart_);
    participants_.erase(participant);
  }

  void addRecentMsg(const chat_message& msg) {
	  WriteLock lock(mutex_);
	  recent_msgs_.push_back(msg);
	  while (recent_msgs_.size() > max_recent_msgs) {
		  recent_msgs_.pop_front();
	  }
  }

  void deliver(const chat_message& msg)
  {
	  auto count = messCount.fetch_add(1, std::memory_order_seq_cst);
	  if (count % 10000 == 0) {
		  std::cout << "Recieved " << count << " messages" << std::endl;
	  }
	  addRecentMsg(msg);
    for (auto participant: participants_)
      participant->deliver(msg);
  }

private:
	void addPart(chat_participant_ptr participant) {
		WriteLock lock(mutexPart_);
		participants_.insert(participant);
	}
private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
  SharedMutex mutex_;
  SharedMutex mutexPart_;
  std::atomic<long long> messCount = 0;

};

//----------------------------------------------------------------------

class chat_server
{
	typedef std::deque<chat_message> MessQueue;
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

	void addCmd(const chat_message& mess) {
		Lock lock(mutex_);
		systemMessQueue_.push_back(mess);
		cond.notify_one();
	}

	bool isStopped() const { return io_service_.stopped(); }
private:
	void do_accept();

	

	void handleMessage() {
		const std::string::size_type start = 3;
		while (!io_service_.stopped()) {
			std::string cmdWithArg;
			{
				Lock lock(mutex_);
				while (systemMessQueue_.empty()) {
					cond.wait(lock);
				}
				chat_message m(systemMessQueue_.front());
				cmdWithArg = m().text().substr(start);
				systemMessQueue_.pop_front();
			}

			std::string::size_type pos = cmdWithArg.find(' ');
			std::string cmd;
			std::string args;
			if (pos != std::string::npos) {
				cmd = cmdWithArg.substr(0, pos);
				args = cmdWithArg.substr(pos + 1);
			}
			else {
				cmd = cmdWithArg;
			}
			if (cmd == "print") {
				std::cout << args << std::endl;
			}
			else if (cmd == "exit") {
				io_service_.stop();
			}
		}

	}
	std::shared_ptr<boost::thread> thread_;
	boost::asio::io_service& io_service_;
	MessQueue systemMessQueue_;
	Mutex mutex_;
	Condition cond;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	chat_room room_;
};

//----------------------------------------------------------------------

class chat_session
  : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
	typedef std::unique_lock<std::mutex> WriteLock;
public:
  chat_session(tcp::socket socket, chat_room& room, chat_server& server)
    : socket_(std::move(socket)),
      room_(room),
	  chat_(server)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg)
  {
      do_write(msg);
  }

private:
  void do_read_header()
  {
	  if (!chat_.isStopped()) {
		  auto self(shared_from_this());
		  boost::asio::async_read(socket_,
			  boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			  [this, self](boost::system::error_code ec, std::size_t /*length*/)
		  {
			  if (!ec)
			  {
				  if (read_msg_.decode_header()) {
					  do_read_body();
				  }
			  }
			  else
			  {
				  room_.leave(shared_from_this());
			  }
		  });
	  }
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
			  if (read_msg_.decode_body()) {
				  const std::string& s = read_msg_().text();
				  //std::cout << "Recieved message:" << s << std::endl;
				  std::string findStr("/c");
				  if (s.size() > findStr.size() && s.substr(0, findStr.size()) == findStr) {
					  chat_.addCmd(read_msg_);
				  }
				  else {
					  room_.deliver(read_msg_);
				  }
			  }
            do_read_header();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }
  typedef std::shared_ptr<std::string> StrPtr;
  void do_write(const chat_message& msg)
  {
	StrPtr s = std::make_shared<std::string>(msg.data(), msg.length());
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(s->data(),
          s->length()),
        [this, self, s](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (ec)
          {
            room_.leave(shared_from_this());
          }
        });
  }
  tcp::socket socket_;
  chat_server& chat_;
  chat_room& room_;
  chat_message read_msg_;
};



void chat_server::do_accept()
{
	acceptor_.async_accept(socket_,
		[this](boost::system::error_code ec)
	{
		if (!ec)
		{
			std::make_shared<chat_session>(std::move(socket_), room_, *this)->start();
		}

		do_accept();
	});
}
//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 3)
    {
      std::cerr << "Usage: chat_server <port>  <amount_of_threads_in_pool>\n";
      return 1;
    }

    boost::asio::io_service io_service;
	boost::asio::io_service::work work(io_service);
	boost::thread_group threads; // thread pool of 10 threads
 	int port = atoi(argv[1]);
	if(port < 1000 || port > 60000){
 		std::cerr << "Error port. Port must be in [1000,60000]" << std::endl;
		return 1;
	}
	int N = atoi(argv[2]);
	if(N <= 0 || N > 100 ){
		std::cerr << "Error threads count. Threads must be in [1,100]" << std::endl;
		return 1;
	}
	for (int i = 0; i < N; ++i){
		threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service)); 
	}

    tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
	chat_server server(io_service, endpoint);

    io_service.run();
	threads.join_all();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
