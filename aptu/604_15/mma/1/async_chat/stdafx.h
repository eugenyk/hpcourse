#pragma once


#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using std::atomic;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::thread;
using std::pair;
using std::make_pair;
using std::vector;

#include <boost\asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;

#include <google\protobuf\io\coded_stream.h>
#include "chat_protocol.pb.h"

using message_ptr = shared_ptr<Message>;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;