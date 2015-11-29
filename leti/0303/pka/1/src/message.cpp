// Project: Chat
//
//  Created on: 14 нояб. 2015 г.
//      Author: motorhead
//

#include <cstring>
#include "message.h"
namespace network {

void message::reserve(uint32_t new_size) {
	if(!data.unique()) {
		std::shared_ptr<char> new_data(new char[new_size]);
		data = new_data;
	} else {
		if(new_size > capacity) {
			std::shared_ptr<char> new_data(new char[new_size]);
			data = new_data;
			capacity = new_size;
		}
	}
	size = new_size;
}

void* message::getData() {
	if(!data.unique()) {
		std::shared_ptr<char> new_data(new char[size]);
		std::memcpy(new_data.get(), data.get(), size);
		data = new_data;
	}
	return data.get();
}

} /* namespace network */
