//============================================================================
// Name        : chat.cpp
// Author      : hardened_steel
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <regex>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <map>
#include <list>
#include <boost/program_options.hpp>
#include "Message.pb.h"
#include "server.h"
#include "client.h"

void run_server(unsigned thread_count, short unsigned port);
void run_client(std::string ip, std::string name, short unsigned port);

int main(int argc, char* argv[]) {
	namespace po = boost::program_options;

	std::string ip_address;
	std::string name;
	short unsigned port;
	unsigned thread_count;

	po::options_description desc("Program options");
	desc.add_options()
			("help,h", "show help")
			("client,c", "start client chat")
			("server,s", "start server chat")
	;

	po::options_description client("Client options");
	client.add_options()
			("ip", po::value<std::string>(&ip_address)->required(), "address of server")
			("port,p", po::value<short unsigned>(&port)->default_value(10000), "server port")
			("name,n", po::value<std::string>(&name), "user name")
	;

	po::options_description server("Server options");
	server.add_options()
			("threads,t", po::value<unsigned>(&thread_count)->default_value(1), "number of threads")
			("port,p", po::value<short unsigned>(&port)->default_value(10000), "listening port")
	;
	try {
		po::variables_map vm;
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		po::notify(vm);

		if(vm.count("help")) {
			std::cout << desc << std::endl;
			std::cout << client << std::endl;
			std::cout << server << std::endl;
			return 0;
		}
		if(vm.count("client") == 1) {
			desc.add(client);
			po::store(po::parse_command_line(argc, argv, desc), vm);
			po::notify(vm);
			run_client(ip_address, name, port);
			google::protobuf::ShutdownProtobufLibrary();
			return 0;
		}
		if(vm.count("server") == 1) {
			desc.add(server);
			po::store(po::parse_command_line(argc, argv, desc), vm);
			po::notify(vm);
			run_server(thread_count, port);
			google::protobuf::ShutdownProtobufLibrary();
			return 0;
		}
	} catch(po::error& e) {
		std::cerr << "error: " << e.what() << std::endl;
		std::cerr << desc << std::endl;
		google::protobuf::ShutdownProtobufLibrary();
		return -1;
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		google::protobuf::ShutdownProtobufLibrary();
		return -1;
	}
	std::cerr << "no option of program, run with --help, for see parameters" << std::endl;
	google::protobuf::ShutdownProtobufLibrary();
	return -1;
}

void run_server(unsigned thread_count, short unsigned port) {
	network::server::tcp server {thread_count, port};
	std::map<std::string, network::server::tcp::connection> clients;
	std::list<std::thread> tasks;

	auto doCommand = [&](std::string sender, std::string command) mutable {
		std::string file_name = "command_" + sender + ".txt";
		command += " > " + file_name;
		std::system(command.c_str());

		std::stringstream stream;
		stream << std::ifstream(file_name).rdbuf();
		std::string result = stream.str();
		result[result.length() - 1] = '\0';

		server::proto::Message answer;
		answer.set_text(result);
		answer.set_sender("/c " + sender);

		network::message message(answer.ByteSize());
		if(answer.SerializeToArray(message.getData(), message.getSize())) {
			std::cout << "[command]: " << result << std::endl;
			for(auto& client: clients) {
				server.transmit(client.second, message);
			}
		} else {
			std::cerr << "error serialize command answer" << std::endl;
		}
	};

	server.onConnect = [&](network::server::tcp::connection&& client) mutable {
		std::string address = client.getAddress();
		std::cout << "client[" << address << "] connected" << std::endl;
		clients.emplace(address, std::move(client)).first->second.receive(network::message{1024});
	};

	server.onDisconnect = [&](network::server::tcp::connection& client) {
		std::string address = client.getAddress();
		clients.erase(address);
		std::cout << "client[" << address << "] disconnected" << std::endl;
	};

	server.onReceive = [&](network::server::tcp::connection& client, network::message message) mutable {
		server::proto::Message chat_message;

		if(chat_message.ParseFromArray(message.getData(), message.getSize())) {
			std::string text = chat_message.text();
			std::string sender = chat_message.sender();

			std::cout << '[' << sender << "]: " << text << std::endl;
			if(std::regex_match(text, std::regex{"/c .*"}) && (text.find("sudo") == std::string::npos)) {
				std::thread th(doCommand, sender, text.substr(3, text.length() - 3));
				tasks.push_back(std::move(th));
			} else {
				for(auto& pair: clients) {
					if(client != pair.second) server.transmit(pair.second, message);
				}
			}
		} else {
			std::cerr << "error deserialize message" << std::endl;
		}
		client.receive(std::move(message));
	};

	for(;;) {
		std::string command;
		std::cin >> command;
		if(command == "quit") break;
		std::cout << "unknow command: \"" << command << "\"" << std::endl;
	}
	for(auto& th: tasks) th.join();
}

void run_client(std::string ip, std::string name, short unsigned port) {
	network::client client(ip, port);

	client.onReceive = [&](network::message message) mutable {
		server::proto::Message chat_message;
		if(chat_message.ParseFromArray(message.getData(), message.getSize())) {
			std::cout << '[' << chat_message.sender() << "]: " << chat_message.text() << std::endl;
			client.receive(std::move(message));
		} else {
			std::cerr << "error deserialize message" << std::endl;
		}
	};

	server::proto::Message chat_message;
	chat_message.set_sender(name);

	std::cout << "connected" << std::endl;
	for(;;) {
		network::message message;
		std::string text;
		std::getline(std::cin, text);

		if(text == "quit") break;
		chat_message.set_text(text);

		message.reserve(chat_message.ByteSize());
		if(chat_message.SerializeToArray(message.getData(), message.getSize())) {
			client.send(message);
		} else {
			std::cerr << "error serialize message" << std::endl;
		}
	}
	std::cout << "disconnected" << std::endl;
}
