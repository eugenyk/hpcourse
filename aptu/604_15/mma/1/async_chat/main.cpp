#include "stdafx.h"

#include "server.h"

using namespace async_chat;

int main(int argc, char* argv[])
{
	const int port = 20053;
	if (argc != 2)
	{
		cout << "Usage: async_chat <thread_count>" << endl;
		return -1;
	}

	int ntrhead = std::atoi(argv[1]);
	if (ntrhead < 1)
	{
		cout << "Wrong number of threads" << endl;
		return -1;
	}

	try
	{
		io_service service(ntrhead);
		
		tcp::endpoint endpoint(tcp::v4(), port);
		task_executor_ptr task_executor(make_shared<task_executor>());
		
		server server(service, endpoint, task_executor);
		server.start();

		vector<thread> runners;
		for (int i = 0; i < ntrhead; ++i)
			runners.emplace_back([&service]() { service.run(); });

		runners.emplace_back([&task_executor]() {task_executor->start(); });

		cout << "Server runned, type anything to stop" << endl;
		string trash;
		cin >> trash;

		service.stop();
		task_executor->stop();
		for (auto& t : runners) t.join();
	}
	catch (std::exception const& e)
	{
		cerr << e.what() << endl;
		return -1;
	}
	return 0;
}