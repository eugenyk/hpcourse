#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include "Message.pb.h"

class chat_message
{
private:
	char* data_ = nullptr;
	size_t body_length_ = 0;
	server::proto::Message mess_;

public:
	enum { header_length = 4 };

	chat_message() : mess_()
	{
		body_length_ = 0;
		data_ = new char[header_length];
	}

	chat_message(const chat_message& m) : mess_(m())
	{
		body_length_ = m.body_length();
		data_ = new char[m.length()];
		std::memcpy(data_, m.data(), m.length());
	}
	
	~chat_message() 
	{
		mess_.Clear();
		del_data();
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	size_t body_length() const
	{
		return body_length_;
	}

	void body_length(size_t new_length)
	{
		body_length_ = new_length;
	}

	bool decode_body() {
		bool res = mess_.ParseFromArray(body(), body_length());
		return res;
	}

	bool encode_body() 
	{
		auto s = mess_.ByteSize();
		body_length(s);
		del_data();
		data_ = new char[length()];
		bool res = mess_.SerializeToArray((void*)body(), s);
		mess_.Clear();
		encode_header();
		return res;
	}

	server::proto::Message& operator()() 
	{
		return mess_;
	}

	const server::proto::Message& operator()()const 
	{
		return mess_;
	}

	bool decode_header()
	{
		auto t = static_cast<decltype(body_length_)>(*data_);
		body_length(t);
		del_data();
		data_ = new char[length()];
		encode_header();
		return true;
	}

	void encode_header()
	{
		std::memcpy(data_, (void*)&body_length_, sizeof(body_length_));
	}

private:
	void del_data() 
	{
		delete[] data_;
		data_ = nullptr;
	}
};

#endif // CHAT_MESSAGE_HPP