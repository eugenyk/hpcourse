# ConsoleChat
Server and client chat implementation for [high performance computing course 2015](https://github.com/eugenyk/hpcourse).

## Build

### Instructions

#### Visual Studio
1. Run ConsoleServer
2. Run ConsoleClient projects

#### Structure
- ConsoleClient
-- Program.cs (Start screen, sending Usernames and messages to Stream, receiving messages from User)
- ConsoleServer
-- ClientObject.cs (Class for a tcpClient(id, username, message),  receive message from Stream)
-- ServerObject.cs (Class for tcpConnections, listening Stream for tcpClients form Server, send messages to Stream to Clients)
-- Program.cs (Starts a Stream)