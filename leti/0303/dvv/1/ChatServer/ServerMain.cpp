#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "ChatServer.h"

using boost::asio::ip::tcp;

void main()
{
	int portNum = 10000;
	while (!(portNum <= 25000 && portNum >= 15000))
	{
		std::cout << "Enter port number between 15000 and 25000\n";
		std::cin >> portNum;
	}

	int threadNum = 10;
	while (!(threadNum <= 30 && threadNum >= 20))
	{
		std::cout << "Enter thread count between 20 and 30\n";
		std::cin >> threadNum;
	}

	try
	{
		boost::asio::io_service io_service;
		boost::asio::io_service::work work(io_service);
		boost::thread_group threads;

		for (int i = 0; i < threadNum; i++)
		{
			threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
		}

		tcp::endpoint endpoint(tcp::v4(), portNum);
		chat_server server(io_service, endpoint);

		io_service.run();
		threads.join_all();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
