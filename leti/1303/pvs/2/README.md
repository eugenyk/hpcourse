#Lab 2 (Chat server)

Connecting to server with URL: ```ws://IP:8081/```

###Message structure

```
Message: {
    sender: <String>,
    receiver: <String>,
    body: <String>
}
```

###Protocol

First message from client after it connected must be with receiver = "server", sender = "CLIENT_NAME".
If CLIENT_NAME isn't "server" and client with same name does not connected, client can send messages.

If receiver = "all", message will send to all connected clients.

Messages are sent in JSON format.