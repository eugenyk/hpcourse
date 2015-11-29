// Project: Chat
//
//  Created on: 14 нояб. 2015 г.
//      Author: motorhead
//

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <inttypes.h>
#include <memory>

namespace network {

class message
{
	std::shared_ptr<char> data;
	uint32_t size, capacity;
public:
	message(): size(0), capacity(0) {}
	message(uint32_t size): data(new char[size]), size(size), capacity(size) {}
	message(message&& other): data(std::move(other.data)), size(other.size), capacity(other.capacity) {}
	message(const message& other): data(other.data), size(other.size), capacity(other.capacity) {}
	message& operator=(message&& other) {
		data = std::move(other.data);
		size = other.size;
		capacity = other.capacity;
		return *this;
	}
	message& operator=(const message&& other) {
		data = other.data;
		size = other.size;
		capacity = other.capacity;
		return *this;
	}

	uint32_t getSize() const { return size; }
	const void* getData() const { return data.get(); }
	void* getData();
	void reserve(uint32_t new_size);
};

} /* namespace network */

#endif /* MESSAGE_H_ */
