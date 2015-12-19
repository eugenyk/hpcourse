#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "chat_message.h"
#include <mutex>

//this is client

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

enum class ClientState {
	STATE_STOPPED,
	STATE_STARTED,
	STATE_NEED_STOP
};

class chat_client
{
	typedef std::unique_lock<std::mutex> WriteLock;
public:
  chat_client(boost::asio::io_service& io_service,
      tcp::resolver::iterator endpoint_iterator,
	  const std::string& name)
    : io_service_(io_service),
      socket_(io_service),
	  name_(name),
	  state(ClientState::STATE_STARTED)
  {
    do_connect(endpoint_iterator);
  }

  void write(const chat_message& msg)
  {
    io_service_.post(
        [this, msg]()
        {
			do_write(msg);
        });
  }

  void close()
  {
    io_service_.post([this]() { socket_.close(); });
  }
  bool isEmpty() {
	
  }
  bool isStopped()const {
	  return state == ClientState::STATE_STOPPED;
  }
  void stop() {
	  state = ClientState::STATE_STOPPED;
  }
private:
  void do_connect(tcp::resolver::iterator endpoint_iterator)
  {
    boost::asio::async_connect(socket_, endpoint_iterator,
        [this](boost::system::error_code ec, tcp::resolver::iterator)
        {
          if (!ec && !isStopped())
          {
            do_read_header();
          }
        });
  }

  void do_read_header()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
			if (isStopped()) return;
          if (!ec)
          {
			  if (read_msg_.decode_header()) {
				  do_read_body();
			  }
          }
          else
          {
			  stop();
			std::cout << ec.category().name() << ':' << ec.value() << " " << ec.message();
            socket_.close();
          }
        });
  }

  void do_read_body()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
			if (isStopped()) return;
          if (!ec)
          {
			  if (read_msg_.decode_body()) {
				  std::cout << read_msg_().sender() << ":\n";
				  std::cout << read_msg_().text() << "\n\n";
				  read_msg_().Clear();
			  }
			  else {
				  std::cout << "\n!client::error in reading body!\n\n";
			  }
            do_read_header();
          }
          else
          {
			  stop();
			std::cout << ec.category().name() << ':' << ec.value() << " " << ec.message();
            socket_.close();
          }
        });
  }
  typedef std::shared_ptr<std::string> StrPtr;
  void do_write(const chat_message& msg)
  {
	  StrPtr s = std::make_shared<std::string>(msg.data(), msg.length());
		boost::asio::async_write(socket_,
			boost::asio::buffer(s->c_str(),
				s->length()),
			[this, s](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (isStopped()) return;
			if (ec){
				stop();
				std::cout << ec.category().name() << ':' << ec.value() << " " << ec.message();
				socket_.close();
			}
		});
  }

private:
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  chat_message read_msg_;
 // chat_message_queue write_msgs_;
  std::string name_;
  std::mutex mutex_;
  ClientState state;
};

int main(int argc, char* argv[])
{
	srand(time(0));
	setlocale(LC_ALL, "Russian");
  try
  {
    if (argc != 4)
    {
      std::cerr << "Usage: chat_client <host> <port> <client_name>\n";
      return 1;
    }
	std::cout << "I'm " << argv[3] << std::endl;
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
    chat_client c(io_service, endpoint_iterator,argv[3]);

	std::thread t([&io_service]() {
		io_service.run();
		std::cout << "Server connection lost. Exit..." << std::endl;
		for( auto i = 0; i < 100000000; i++) {
			i++;
			i--;
		}
		exit(0);
	});

    std::string line;
    //while (getline(std::cin, line))
	for (int i = 0; i < 1000000; i++)
    {
      chat_message msg;
	  if (i % 100 == 0) {
		  line.clear();
	  }
	  line.push_back(std::rand() % ('z'-'a') + 'a');
	  msg().set_text(line);
	  msg().set_sender(argv[3]);
	  if (msg.encode_body()) {
		  c.write(msg);
	  }
	  else {
		  std::cout << "Error" << std::endl;
	  }
    }
	//while (!c.isEmpty()) {
		//Sleep(100000);
	std::cout << "All write" << std::endl;
	//system("pause");
char ccc;
std::cin >> ccc;
	//}
    c.close();
    t.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
