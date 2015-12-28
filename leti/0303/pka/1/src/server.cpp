#include <boost/endian/buffers.hpp>
#include <mutex>
#include "server.h"

namespace network {

void server::run() {
	for(auto i = thread_count; i > 0; --i) {
		auto process = [](boost::asio::io_service& service) {
			service.run();
		};
		std::thread th(process, std::ref(service));
		threads.push_back(std::move(th));
	}
}

void server::stop() {
	service.stop();
	for(auto& thread: threads) thread.join();
	threads.clear();
}

class server::tcp::connection::implementation: public std::enable_shared_from_this<server::tcp::connection::implementation>
{
	friend class server::tcp;
	friend class server::tcp::connection;
	boost::asio::ip::tcp::socket socket;
	std::mutex send_lock, receive_lock;
	server::tcp* parent;
public:
	implementation(boost::asio::io_service& service, server::tcp* parent): socket(service), parent(parent) {}
	~implementation() {
		boost::system::error_code ec;
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		socket.close(ec);
	}
public:
	void send(const message message) {
		using namespace boost;
		send_lock.lock();

		server::tcp::connection client(this->shared_from_this());
		std::shared_ptr<endian::big_uint32_buf_t> size = std::make_shared<endian::big_uint32_buf_t>(message.getSize());
		auto handler = [=](const system::error_code& error, std::size_t) mutable {
			if(!error) {
				auto handler = [=](const system::error_code& error, std::size_t size) mutable {
					send_lock.unlock();
					if(!error) parent->onSend(client, std::move(message));
					else parent->onDisconnect(client);
				};
				asio::async_write(socket, asio::buffer(message.getData(), size->value()), handler);
			} else {
				send_lock.unlock();
				parent->onDisconnect(client);
			}
		};
		asio::async_write(socket, asio::buffer(size.get(), sizeof(endian::big_uint32_buf_t)), handler);
	}
	void receive(message message) {
		using namespace boost;
		receive_lock.lock();

		server::tcp::connection client(this->shared_from_this());
		std::shared_ptr<endian::big_uint32_buf_t> size {new endian::big_uint32_buf_t};
		auto handler = [=](const system::error_code& error, std::size_t) mutable {
			if(!error) {
				message.reserve(size->value());
				auto buffer = asio::buffer(message.getData(), size->value());
				auto handler = [=](const system::error_code& error, std::size_t) mutable {
					receive_lock.unlock();
					if(!error) parent->onReceive(client, std::move(message));
					else parent->onDisconnect(client);
				};
				asio::async_read(socket, std::move(buffer), handler);
			} else {
				receive_lock.unlock();
				parent->onDisconnect(client);
			}
		};
		asio::async_read(socket, asio::buffer(size.get(), sizeof(endian::big_uint32_buf_t)), handler);
	}
};

server::tcp::tcp(unsigned thread_count, short unsigned port)
: server(thread_count, port), endpoint(boost::asio::ip::tcp::v4(), port), acceptor(service, endpoint) {
	doConnect();
	server::run();
}

void server::tcp::doConnect() {
	using namespace boost;

	std::shared_ptr<server::tcp::connection::implementation> client = std::make_shared<server::tcp::connection::implementation>(service, this);
	auto handler = [client, this](const boost::system::error_code& error) {
		if(!error) {
			onConnect(connection{client});
			doConnect();
		}
	};
	acceptor.async_accept(client->socket, handler);
}

server::tcp::~tcp() {
	acceptor.close();
	stop();
}

server::tcp::connection::~connection() {
}

std::string server::tcp::connection::getAddress() const {
	std::string address = impl->socket.remote_endpoint().address().to_string();
	address += std::to_string(impl->socket.remote_endpoint().port());
	return std::move(address);
}

bool server::tcp::connection::operator==(const connection& other) const {
	return impl == other.impl;
}

void server::tcp::connection::send(const message& message) {
	impl->send(message);
}

void server::tcp::connection::receive(message&& message) {
	impl->receive(std::move(message));
}

} /* namespace network */
