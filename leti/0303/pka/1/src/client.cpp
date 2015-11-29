#include <boost/endian/buffers.hpp>
#include "client.h"

namespace network {

client::client(std::string ip, short unsigned port): socket(service), endpoint(boost::asio::ip::address::from_string(ip), port) {
	auto process = [this](boost::asio::io_service& service) {
		receive(network::message{1024});
		service.run();
	};
	socket.connect(endpoint);
	this->process = std::thread(process, std::ref(service));
}

void client::receive(network::message message) {
	using namespace boost;
	receive_lock.lock();

	std::shared_ptr<endian::big_uint32_buf_t> size {new endian::big_uint32_buf_t};
	auto handler = [=](const system::error_code& error, std::size_t) mutable {
		if(!error) {
			message.reserve(size->value());
			auto buffer = asio::buffer(message.getData(), size->value());
			auto handler = [=](const system::error_code& error, std::size_t) mutable {
				receive_lock.unlock();
				if(!error) onReceive(std::move(message));
				else service.stop();
			};
			asio::async_read(socket, std::move(buffer), handler);
		} else {
			receive_lock.unlock();
			service.stop();
		}
	};
	asio::async_read(socket, asio::buffer(size.get(), sizeof(endian::big_uint32_buf_t)), handler);
}

void client::send(const message message) {
	using namespace boost;
	send_lock.lock();

	std::shared_ptr<endian::big_uint32_buf_t> size = std::make_shared<endian::big_uint32_buf_t>(message.getSize());
	auto handler = [=](const system::error_code& error, std::size_t) mutable {
		if(!error) {
			auto handler = [=](const system::error_code& error, std::size_t size) mutable {
				send_lock.unlock();
				if(!error) onSend(std::move(message));
				else service.stop();
			};
			asio::async_write(socket, asio::buffer(message.getData(), size->value()), handler);
		} else {
			send_lock.unlock();
			service.stop();
		}
	};
	asio::async_write(socket, asio::buffer(size.get(), sizeof(endian::big_uint32_buf_t)), handler);
}

client::~client() {
	service.stop();
	process.join();
}

} /* namespace network */
