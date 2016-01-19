#include "stdafx.h"

#include "client.h"
#include "room.h"
#include "task_executor.h"

using namespace async_chat;

client::client(tcp::socket socket, room_ptr room, task_executor_ptr task_executor)
	: write_runned_(false)
	, socket_(std::move(socket))
	, room_(room)
	, task_executor_(task_executor)
{
}

void client::post_message(message_ptr msg)
{
	message_queue_.push(msg);
	bool exp = false;
	if (write_runned_.compare_exchange_strong(exp, true))
		do_write();
}

void client::start()
{
	buffer_.resize(max_header_length_);
	bytes_readed_ = 0;
	read_header();
}

void client::read_header()
{
	auto self(shared_from_this());
	if (bytes_readed_)
	{
		CodedInputStream is(buffer_.data(), bytes_readed_);
		if (is.ReadVarint32(&message_length_))
		{
			header_length_ = is.CurrentPosition();
			read_message();
			return;
		} 
		else if (max_header_length_ == bytes_readed_)
		{
			room_->disconnect(self);
			return;
		}
	}
	boost::asio::async_read(socket_,
		boost::asio::buffer(buffer_.data() + bytes_readed_, max_header_length_ - bytes_readed_),
		[this, self](error_code ec, size_t len)
	{
		if (ec)
		{
			room_->disconnect(self);
			return;
		}
		bytes_readed_ += len;
		read_header();
	});
}

void client::read_message()
{
	auto self(shared_from_this());
	if (buffer_.size() < message_length_ + header_length_)
		buffer_.resize(message_length_ + header_length_);
	if (bytes_readed_ >= message_length_ + header_length_)
	{
		auto message(make_shared<Message>());
		if (!message->ParseFromArray(buffer_.data() + header_length_, message_length_))
		{
			room_->disconnect(self);
			return;
		}
		dispatch_message(message);

		size_t extra_bytes = bytes_readed_ - message_length_ - header_length_;
		for (size_t i = 0; i < extra_bytes; ++i)
		{
			buffer_[i] = buffer_[i + header_length_ + message_length_];
		}
		bytes_readed_ = extra_bytes;
		read_header();
		return;
	}

	boost::asio::async_read(socket_,
		boost::asio::buffer(buffer_.data() + bytes_readed_, message_length_ + header_length_ - bytes_readed_),
		[this, self](error_code ec, size_t len)
	{
		if (ec)
		{
			room_->disconnect(self);
			return;
		}
		bytes_readed_ += len;
		read_message();
	});
}

void client::dispatch_message(message_ptr message)
{
	if (message->type() == Message::COMMAND)
	{
		if (message->text().size() != 0)
		{
			task_executor_->add_task(shared_from_this(), std::move(*message->text().begin()));
		}
	}
	else
	{
		room_->broadcast(shared_from_this(), message);
	}
}

void client::do_write()
{
	auto self(shared_from_this());

	if (local_messages_.empty())
		local_messages_ = message_queue_.move_to_queue();

	if (local_messages_.empty())
	{
		write_runned_.store(false);
		bool exp = false;
		local_messages_ = message_queue_.move_to_queue();
		if (local_messages_.empty() || !write_runned_.compare_exchange_strong(exp, true))
		{
			return;
		}
	}

	auto buf = make_shared<vector<uint8_t>>(write_buffer_length_);
	size_t wroten_bytes = 0;

	while (!local_messages_.empty() && wroten_bytes < write_buffer_length_)
	{
		auto message = local_messages_.front();
		size_t size = message->ByteSize();
		size_t req = size + wroten_bytes + max_header_length_;
		if (req < write_buffer_length_ || wroten_bytes == 0)
		{
			local_messages_.pop();
			if (buf->size() < req)
				buf->resize(req);
			auto pos = CodedOutputStream::WriteVarint32ToArray(size, buf->data() + wroten_bytes);
			wroten_bytes += pos - (buf->data() + wroten_bytes);
			message->SerializeToArray(pos, size);
			wroten_bytes += size;
		} 
		else
		{
			break;
		}
	}
	
	boost::asio::async_write(socket_,
		boost::asio::buffer(buf->data(), wroten_bytes),
		[this, buf, self, wroten_bytes](error_code ec, size_t s)
	{
		if (ec)
		{
			room_->disconnect(self);
			return;
		}
		do_write();
	});
}