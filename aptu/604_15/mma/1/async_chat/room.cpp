#include "stdafx.h"

#include "room.h"

using namespace async_chat;

void room::connect(client_ptr client)
{
	clients_.put(client);
}

void room::disconnect(client_ptr client)
{
	clients_.erase(client);
}

void room::broadcast(client_ptr author, message_ptr message)
{
	auto post = [author, message](client_ptr client)
	{
		if (client != author)
			client->post_message(message);
	};
	clients_.foreach_unordered(post);
}
