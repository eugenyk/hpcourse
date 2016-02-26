
#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Message.pb.h"

class chat_message: public server::proto::Message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0),decoded_data_(""),encoded_(false),decoded_(false)
	{
	}

	bool encoded()
	{
		return encoded_;
	}

	bool decoded()
	{
		return decoded_;
	}

	const char* mdata() const
	{
		return mdata_;
	}

	char* mdata()
	{
		return mdata_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return mdata_ + header_length;
	}

	char* body()
	{
		return mdata_ + header_length;
	}

	size_t body_length() const
	{
		return body_length_;
	}

	void body_length(size_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	bool decode_header()
	{
		using namespace std; // For strncat and atoi.
		char header[header_length + 1] = "";
		strncat(header, mdata_, header_length);
		body_length_ = atoi(header);
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		using namespace std; // For sprintf and memcpy.
		char header[header_length + 1] = "";
		sprintf(header, "%4d", body_length_);
		memcpy(mdata_, header, header_length);
	}

	void encode()
	{
		encoded_ = false;
		std::string msg = this->SerializeAsString();
		body_length(msg.length());
		memcpy(body(), msg.c_str(), body_length());
		encode_header();
		encoded_ = true;
	}

	void decode()
	{
		bool decoded_header_ = decode_header();
		if (decoded_header_)
		{
			decoded_ = ParseFromArray(mdata_ + header_length, body_length_);
			if (decoded_)
			{
				decoded_data_ = sender() + ": " + text();
			}
		}
		else
		{
			decoded_data_ = "--error: header is broken .( ";
		}
	}

	std::string mtext()
	{
		return decoded_data_;
	}

private:
	char mdata_[header_length + max_body_length];
	std::string decoded_data_;
	size_t body_length_;
	bool encoded_;
	bool decoded_;
};
#endif
