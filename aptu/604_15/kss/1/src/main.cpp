#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <google/protobuf/io/coded_stream.h>
#include "message.pb.h"

/*******************************************************************
 * Configuration****************************************************
********************************************************************/
static std::string const CHAT_SERVER_AUTOR = "Chat server";
#ifdef DEBUG
static size_t const INITIAL_BUFFER_SIZE = 1 << 2; // 4 b
#else
static size_t const INITIAL_BUFFER_SIZE = 1 << 12; // 4 Kb
#endif
static size_t const MESSAGES_IN_QUEUE_TO_FALLBACK = 1 << 8; // 256
static size_t const MESSAGES_SUM_SIZE_TO_FALLBACK = 1 << 12; // 4 Kb
static size_t const MAX_VARINT_BYTES = 10;
/*******************************************************************/


using namespace boost::asio;
using ru::spbau::chat::commons::protocol::Message;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::uint8;
using google::protobuf::uint32;

typedef std::pair<size_t, std::shared_ptr<uint8>> serialized_message;

class chat_user
{
public:
    virtual void accept_message_size() = 0;
    virtual bool deliver_message(serialized_message const &message) = 0;
};
typedef std::shared_ptr<chat_user> user_ptr;

serialized_message serialize_message(Message const &message)
{
    size_t msg_size = message.ByteSize();
    size_t size = msg_size + MAX_VARINT_BYTES;
    std::shared_ptr<uint8> buffer(new uint8[size]);
    auto it = CodedOutputStream::WriteVarint32ToArray(msg_size, buffer.get());
    bool written = message.SerializeToArray(it, msg_size);
    assert(written);
    return serialized_message(msg_size + (it - buffer.get()), buffer);
}

class chat_room
{
public:
    chat_room()
        :
          command_executor(1)
    {
    }

    void add_new_user(user_ptr user)
    {
        rw_mutex.lock();
        users.push_back(user);
        rw_mutex.unlock();
#ifdef DEBUG
        std::cout << "User connected" << std::endl;
#endif
        user->accept_message_size();
    }

    void on_user_leave(user_ptr user)
    {
        rw_mutex.lock();
        auto pos = std::find_if(users.begin(), users.end(), [user](user_ptr &ptr)
        {
            return ptr.get() == user.get();
        });
        if (pos != users.end())
        {
            users.erase(pos);
        }
        rw_mutex.unlock();
#ifdef DEBUG
        std::cout << "User leaved" << std::endl;
#endif
    }

    size_t deliver_message_to_all(Message const &message, chat_user const *from_user)
    {
        serialized_message message_serialized(serialize_message(message));
        size_t overflow_queues = 0;
        rw_mutex.lock_shared();
        for(auto &user_p: users)
        {
            if (user_p.get() == from_user)
            {
                continue;
            }
            overflow_queues += user_p->deliver_message(message_serialized);
        }
        rw_mutex.unlock_shared();
        return overflow_queues;
    }

    void execute_command(std::string cmd, user_ptr user_to_send_result)
    {
        command_executor.submit([user_to_send_result, cmd]()
        {
            Message result;
            result.set_type(Message::MESSAGE);
            result.set_author(CHAT_SERVER_AUTOR);

            std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
            char buffer[128];
            std::string result_str;
            if (pipe)
            {
                while (!feof(pipe.get())) {
                    if (fgets(buffer, 128, pipe.get()) == NULL)
                    {
                        break;
                    }
                    result_str += buffer;
                    if (result_str.back() == '\n')
                    {
                        result_str.pop_back();
                        result.add_text(std::move(result_str));
                        result_str.clear();
                    }
                }
                if (result_str.size())
                {
                    result.add_text(result_str);
                }
            } else {
                result.add_text("Command '" + cmd + "'' execution failed!");
            }

            user_to_send_result->deliver_message(serialize_message(result));
        });
    }

private:
    std::vector<user_ptr> users;
    boost::shared_mutex rw_mutex;
    boost::executors::basic_thread_pool command_executor;
};

class user :
        public chat_user,
        public std::enable_shared_from_this<user>
{
public:
    user(ip::tcp::socket sock, chat_room &chat, io_service &service)
        :
            socket(std::move(sock)),
            chat(chat),
            service(service),
            message_buffer(std::max(INITIAL_BUFFER_SIZE, MAX_VARINT_BYTES)),
            buffer_offset(0),
            buffer_red(0),
            write_buffer(std::max(INITIAL_BUFFER_SIZE, MAX_VARINT_BYTES)),
            messages_sum_size(0),
            write_in_progress(false)
    {
    }

    void accept_message_size()
    {
        check_enougth_space(MAX_VARINT_BYTES);

        if (buffer_red)
        {
            bool read_succ;
            size_t read_bytes_count;
            uint32 message_size;
            {
                CodedInputStream input(message_buffer.data() + buffer_offset, buffer_red);
                read_succ = input.ReadVarint32(&message_size);
                read_bytes_count = input.CurrentPosition();
            }
            if (read_succ)
            {
                buffer_offset += read_bytes_count;
                buffer_red -= read_bytes_count;
                accept_message(message_size);
                return;
            }
            if (buffer_red > MAX_VARINT_BYTES)
            {
#ifdef DEBUG
                std::cout << "invalid varint: " << std::endl;
#endif
                chat.on_user_leave(shared_from_this());
                return;
            }
        }

        auto ancor(shared_from_this());
        async_read(socket, buffer(message_buffer.data() + buffer_offset + buffer_red,
                                  message_buffer.size() - buffer_offset - buffer_red),
            transfer_at_least(1),
            [this, ancor](boost::system::error_code ec, std::size_t bytes_red)
            {
#ifdef DEBUG
                std::cout << "Reading message size: bytes red "
                          << buffer_red + bytes_red << std::endl;
#endif
                if (ec)
                {
#ifdef DEBUG
                    std::cout << "error on reading message_size: " << ec.message() << std::endl;
#endif
                    chat.on_user_leave(shared_from_this());
                    return;
                } else {
                    buffer_red += bytes_red;
                    accept_message_size();
                }
            });
    }

    void accept_message(uint32 message_size)
    {
        check_enougth_space(message_size);

        if (buffer_red >= message_size)
        {
            Message message;
            try
            {
                message.ParseFromArray(message_buffer.data() + buffer_offset,
                                       message_size);
            } catch (std::exception &) {
    #ifdef DEBUG
                std::cout << "Failed to parse protobuf message" << std::endl;
    #endif
                chat.on_user_leave(shared_from_this());
                return;
            }
            buffer_offset += message_size;
            buffer_red -= message_size;

#ifdef DEBUG
            std::cout << "message red ["
                      << (message.has_type() ? message.type() : -1) << ", "
                      << (message.has_author() ? message.author() : "no author") << ", "
                      << (message.text_size() ? message.text(0) : "no text")
                      << "]" << std::endl;
#endif
            if (message.type() == Message::COMMAND)
            {
                chat.execute_command(message.text(0), shared_from_this());
            } else {
                size_t busy_factor = chat.deliver_message_to_all(message, this);
                for (size_t idx = 0; idx < busy_factor; ++idx)
                {
                    if (!service.poll_one())
                    {
                        break;
                    }
                }
            }
            accept_message_size();
            return;
        }


        auto ancor(shared_from_this());
        async_read(socket, buffer(message_buffer.data() + buffer_offset + buffer_red,
                                  message_buffer.size() - buffer_offset - buffer_red),
            transfer_at_least(message_size - buffer_red),
            [this, ancor, message_size](boost::system::error_code ec, std::size_t bytes_red)
            {
#ifdef DEBUG
                std::cout << "Reading message: bytes red "
                          << buffer_red + bytes_red << " from "
                          << message_size << std::endl;
#endif
                if (ec)
                {
#ifdef DEBUG
                    std::cout << "error: " << ec.message() << std::endl;
#endif
                    chat.on_user_leave(shared_from_this());
                    return;
                } else {
                    buffer_red += bytes_red;
                    accept_message(message_size);
                }
            });
    }

    bool deliver_message(serialized_message const &message)
    {
        boost::lock_guard<boost::mutex> guard(deliver_queue_mutex);
        messages_to_deliver.push(message);
        if (!write_in_progress)
        {
            write_in_progress = true;
            auto ancor(shared_from_this());
            service.post([this, ancor](){do_write();});
        }
        messages_sum_size += message.first;
        return messages_to_deliver.size() >= MESSAGES_IN_QUEUE_TO_FALLBACK ||
               messages_sum_size >= MESSAGES_SUM_SIZE_TO_FALLBACK;
    }

    ~user()
    {
#ifdef DEBUG
        std::cout << "user destroyed" << std::endl;
#endif
    }

private:
    void check_enougth_space(size_t space_needed)
    {
        assert(buffer_offset + buffer_red <= message_buffer.size());
        if (!buffer_red)
        {
            buffer_offset = 0;
        }
        if (buffer_offset + space_needed <= message_buffer.size())
        {
            return;
        }
        memmove(message_buffer.data(), message_buffer.data() + buffer_offset, buffer_red);
        buffer_offset = 0;
        if (space_needed > message_buffer.size())
        {
            message_buffer.resize(space_needed);
        }
    }

    void do_write()
    {
        boost::lock_guard<boost::mutex> guard(deliver_queue_mutex);
        size_t min_size = messages_to_deliver.front().first;
        if (write_buffer.size() < min_size)
        {
            write_buffer.resize(min_size);
        }
        size_t write_buffer_offset = 0;
        while (!messages_to_deliver.empty())
        {
            serialized_message const &message = messages_to_deliver.front();
            if (message.first > write_buffer.size() - write_buffer_offset)
            {
                break;
            }
#ifdef DEBUG
            std::cout << "message written" << std::endl;
#endif
            memcpy(write_buffer.data() + write_buffer_offset, message.second.get(), message.first);
            write_buffer_offset += message.first;
            messages_sum_size -= message.first;
            messages_to_deliver.pop();
        }


        auto ancor(shared_from_this());
        async_write(socket, buffer(write_buffer.data(), write_buffer_offset),
                    [this, ancor](boost::system::error_code ec, std::size_t)
        {
            if (ec)
            {
#ifdef DEBUG
                std::cout << "error writing: " << ec.message() << std::endl;
#endif
                chat.on_user_leave(shared_from_this());
                return;
            } else {
                {
                    boost::lock_guard<boost::mutex> guard(deliver_queue_mutex);
                    if (messages_to_deliver.empty())
                    {
                        write_in_progress = false;
                        return;
                    }
                }
                do_write();
            }
        });

    }

private:
    ip::tcp::socket socket;
    chat_room &chat;
    io_service &service;
    std::vector<uint8> message_buffer;
    size_t buffer_offset;
    size_t buffer_red;
    std::vector<uint8> write_buffer;
    std::queue<serialized_message> messages_to_deliver;
    boost::mutex deliver_queue_mutex;
    size_t messages_sum_size;
    bool write_in_progress;
};

class connection_handler
{
public:
    connection_handler(io_service &service, int port, chat_room &chat)
        :
          sock(service),
          service(service),
          acc(service, ip::tcp::endpoint(ip::tcp::v4(), port)),
          chat(chat)
    {
    }

public:
    void accept_new_connection()
    {
        acc.async_accept(sock, [this](boost::system::error_code)
        {
            if (sock.is_open())
            {
                sock.set_option(ip::tcp::no_delay(true));
                chat.add_new_user(std::make_shared<user>(std::move(sock), chat, service));
            }
            accept_new_connection();
        });
    }

private:
    ip::tcp::socket sock;
    io_service &service;
    ip::tcp::acceptor acc;
    chat_room &chat;
};

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cout
                << "Usage: ./chat_server $PORT_NUMBER$ $CONCURRENCY_LEVEL$"
                << std::endl
                << "Where $CONCURRENCY_LEVEL$ - number of the worker threads"
                << std::endl;
        return -1;
    }
    size_t port = std::atoi(argv[1]);
    size_t concurrency_level = std::atoi(argv[2]);

    try
    {
        std::cout << "Starting chat server on port "
                  << port
                  << " with concurrency level "
                  << concurrency_level
                  << " ..."
                  << std::endl;

        io_service service(concurrency_level);
        chat_room chat;

        connection_handler handler(service, port, chat);
        handler.accept_new_connection();

        boost::thread_group workers;
        auto const worker =
                boost::bind(&io_service::run, &service);
        for (size_t idx = 0; idx < concurrency_level; ++idx)
        {
            workers.create_thread(worker);
        }

        std::cout << "The server is started" << std::endl;
        std::cout << "Press 'enter' to stop the server"
                  << std::endl;
        std::cin.get();
        std::cout << "Stopping the server..." << std::endl;
        service.stop();
        workers.join_all();
        std::cout << "The server is stopped" << std::endl;
    } catch(std::exception &e) {
        std::cerr << "An exception occured with ";
        if (e.what()) {
            std::cerr << "message: \"" << e.what() << '"';
        } else {
            std::cerr << "no message";
        }
        std::cerr << std::endl;
        google::protobuf::ShutdownProtobufLibrary();
        return -1;
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

