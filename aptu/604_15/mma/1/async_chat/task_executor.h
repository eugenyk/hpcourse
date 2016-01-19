#pragma once

#include "client.h"

namespace async_chat
{

struct task_executor : std::enable_shared_from_this<task_executor>
{
	task_executor();
	void start();
	void stop();
	void add_task(client_ptr, string command);

private:
	using command = pair<client_ptr, string>;

	void do_work();

	io_service io_service_;
	boost::asio::deadline_timer timer_;
	shared_ptr<io_service::work> work_;

	atomic<bool> runned_;
	concurrent_queue<command> tasks_;
	std::queue<command> local_tasks_;
};

using task_executor_ptr = shared_ptr<task_executor>;

} //async_chat