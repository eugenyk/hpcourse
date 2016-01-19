#pragma once

#include "room.h"
#include "task_executor.h"

namespace async_chat
{

struct server
{
	server(io_service&, tcp::endpoint const&, task_executor_ptr);
	void start();

private:
	void do_accept();

	room_ptr room_;
	task_executor_ptr task_executor_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

} //async_chat