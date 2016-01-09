#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Message.pb.h"
#include <algorithm>

class chat_message
{
	
public:
  typedef server::proto::Message Message;
  typedef std::int32_t SizeT;
  enum { header_length = 4 };

  chat_message()
    : body_length_(0)
	  , data_(new char[header_length])
	 , mess_()
  {
  }
  chat_message(const chat_message& m) 
	  : data_(nullptr)
		, body_length_ (m.body_length())
		, mess_(m()) 
  {
	  data_ = new char[m.length()];
	  std::memcpy(data_, m.data(), m.length());
  }
  chat_message(chat_message&& m) = delete;
  chat_message& operator = (chat_message& m) = delete;
  chat_message& operator = (chat_message&& m) = delete;
  ~chat_message() {
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

  SizeT length() const
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

  SizeT body_length() const
  {
    return body_length_;
  }

  void body_length(SizeT new_length)
  {
    body_length_ = new_length;
  }

  bool decode_body() {
	  bool res = mess_.ParseFromArray(body(), body_length());
	  //del_data();
	  return res;
  }

  bool encode_body() {
	  auto s = mess_.ByteSize();
	  body_length(s);
	  del_data();
	  data_ = new char[length()];
	  bool res = mess_.SerializeToArray((void*)body(),s);
	  mess_.Clear();
	  encode_header();
	  return res;
  }

  Message& operator()() {
	  return mess_;
  }
  const Message& operator()()const  {
	  return mess_;
  }
  Message& mess() {
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
	  void del_data() {
			delete [] data_;
			data_ = nullptr;
	  }
private:
  char* data_ = nullptr;
  SizeT body_length_ = 0;
  Message mess_;
};

#endif // CHAT_MESSAGE_HPP