#pragma once

#include "concurrent.h"

namespace async_chat
{

struct room;
using room_ptr = shared_ptr<room>;

struct task_executor;
using task_executor_ptr = shared_ptr<task_executor>;

struct client : std::enable_shared_from_this<client>
{
	client(tcp::socket, room_ptr, task_executor_ptr);
	void start();
	void post_message(message_ptr message);
	
private:

	void read_header();
	void read_message();

	void dispatch_message(message_ptr);

	void do_write();

	vector<uint8_t> buffer_;
	size_t bytes_readed_;
	size_t header_length_;
	size_t message_length_;
	const size_t max_header_length_ = 5;

	const size_t write_buffer_length_ = 16384;

	atomic<bool> write_runned_;

	tcp::socket socket_;
	room_ptr room_;
	task_executor_ptr task_executor_;
	concurrent_queue<message_ptr> message_queue_;
	std::queue<message_ptr> local_messages_;
};

using client_ptr = shared_ptr<client>;

} //async_chat