#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <mutex>
#include "ChatClient.h"

using boost::asio::ip::tcp;

void main()
{
	char* arr[3];
	char ip[15];
	char port[5];
	char name[15];

	std::cout << "Enter host ip\n";
	std::cin >> ip;
	arr[0] = ip;
	//std::cout << ip;

	std::cout << "Enter port number between 15000 and 25000\n";
	std::cin >> port;
	arr[1] = port;
	//std::cout << port;

	std::cout << "Enter your nickname\n";
	std::cin >> name;
	arr[2] = name;
	//std::cout << name;

	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ arr[0], arr[1] });
		chat_client c(io_service, endpoint_iterator, arr[2]);

		std::thread t([&io_service]() { io_service.run(); });

		std::string line;
		while (getline(std::cin, line))
		{
			chat_message msg;
			msg().set_text(line);
			msg().set_sender(arr[2]);
			if (msg.encode_body()) c.write(msg);
			else std::cout << "Encoding error!" << std::endl;
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
