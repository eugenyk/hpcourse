#include "stdafx.h"

#include <fstream>

#include "task_executor.h"

using namespace async_chat;

task_executor::task_executor()
	: io_service_()
	, timer_(io_service_)
	, work_(make_shared<io_service::work>(io_service_))
	, runned_(false)
	, tasks_()
{
}

void task_executor::start()
{
	io_service_.run();
}

void task_executor::stop()
{
	work_.reset();
	io_service_.stop();
}

void task_executor::add_task(client_ptr client, string command)
{
	tasks_.push(make_pair(client, std::move(command)));
	bool exp = false;
	if (runned_.compare_exchange_strong(exp, true))
		do_work();
}

void task_executor::do_work()
{
	if (local_tasks_.empty())
		local_tasks_ = tasks_.move_to_queue();

	if (local_tasks_.empty())
	{
		runned_.store(false);
		bool exp = false;
		local_tasks_ = tasks_.move_to_queue();
		if (local_tasks_.empty() || !runned_.compare_exchange_strong(exp, true))
			return;
	}
	auto self(shared_from_this());

	io_service_.dispatch([this, self]()
	{
		command com = local_tasks_.front();
		local_tasks_.pop();
		string ex_line = com.second;
		int handle = _fileno(_popen(ex_line.c_str(), "r"));
		std::ifstream in(_fdopen(handle, "r"));

		auto result(make_shared<Message>());
		
		result->set_author("SERVER");
		
		result->set_type(Message::COMMAND);

		string cur_line;
		while (!in.eof() && std::getline(in, cur_line))
		{
			result->add_text(std::move(cur_line));
		}

		com.first->post_message(result);

		do_work();
	});
}