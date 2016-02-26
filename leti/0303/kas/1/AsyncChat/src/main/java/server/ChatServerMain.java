package server;

import server.api.ChatServer;

import java.io.IOException;
import java.net.InetSocketAddress;

/**
 * Chat server starter class.
 * First argument is number of threads (default = 10),
 * second argument is host port (default = 10000).
 */
public class ChatServerMain {

    public static void main(String[] args) throws IOException, InterruptedException {
        int nThreads, port;
        try {
            nThreads = Integer.parseInt(args[0]);
        } catch (Exception e) {
            nThreads = 10;
        }
        try {
            port = Integer.parseInt(args[1]);
        } catch (Exception e) {
            port = 10000;
        }

        ChatServer s = new ChatWebSocketServer(new InetSocketAddress("127.0.0.1", port), "server", nThreads);
        s.startServer();
    }
}
