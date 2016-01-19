#pragma once

#include "concurrent.h"
#include "client.h"

namespace async_chat
{

struct room
{
	void connect(client_ptr client);

	void disconnect(client_ptr client);

	void broadcast(client_ptr author, message_ptr message);

private:
	concurrent_set<client_ptr> clients_;
};

} //async_chat