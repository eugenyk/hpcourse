package server.api;

import java.io.IOException;


public interface ChatServer {
    void startServer() throws IOException, InterruptedException;
    void stopServer() throws IOException;
    void restartServer() throws IOException, InterruptedException;
    ChatHandler getHandler();
}
