#include "stdafx.h"

#include "server.h"

using namespace async_chat;

server::server(io_service& io_service, tcp::endpoint const& endpoint, task_executor_ptr task_executor)
	: room_(make_shared<room>())
	, task_executor_(task_executor)
	, acceptor_(io_service, endpoint)
	, socket_(io_service)
{
}

void server::start()
{
	do_accept();
}

void server::do_accept()
{
	auto accept = [this](error_code ec)
	{
		if (!ec)
		{
			client_ptr c = make_shared<client>(std::move(socket_), room_, task_executor_);
			room_->connect(c);
			c->start();
		}
		do_accept();
	};

	acceptor_.async_accept(socket_, accept);
}