// Project: Chat
//
//  Created on: 14 нояб. 2015 г.
//      Author: motorhead
//

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <inttypes.h>
#include <memory>
#include <iostream>

namespace network {

class message
{
	std::shared_ptr<char> data;
	uint32_t size, capacity;
public:
	message(): size(0), capacity(0) { /*std::cout << "constructor empty message" << std::endl;*/ }
	message(uint32_t size): data(new char[size], std::default_delete<char[]>()), size(size), capacity(size) { /*std::cout << "constructor " << data.get() << " message(" << size << ")" << std::endl;*/ }
	message(message&& other): data(std::move(other.data)), size(other.size), capacity(other.capacity) { /*std::cout << "move constructor " << data.get() << " message(" << size << ")" << std::endl;*/ }
	message(const message& other): data(other.data), size(other.size), capacity(other.capacity) { /*std::cout << "copy constructor " << data.get() << " message(" << size << ")" << std::endl;*/ }
	message& operator=(message&& other) {
		/*std::cout << "move assign " << data.get() << " message(" << size << ") = " << other.getData() << " (" << other.size << ")" << std::endl;*/
		data = std::move(other.data);
		size = other.size;
		capacity = other.capacity;
		return *this;
	}
	message& operator=(const message&& other) {
		data = other.data;
		size = other.size;
		capacity = other.capacity;
		/*std::cout << "copy assign " << data.get() << " message(" << size << ")" << std::endl;*/
		return *this;
	}

	uint32_t getSize() const { return size; }
	const void* getData() const { return data.get(); }
	void* getData();
	void reserve(uint32_t new_size);
	~message() { /*if(data) std::cout << "destructor " << data.get() << " message(" << size << ")" << std::endl;*/ }
};

} /* namespace network */

#endif /* MESSAGE_H_ */
