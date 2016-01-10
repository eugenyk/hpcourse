package ru.spb.leti.garadagly.chat.server;

import ru.spb.leti.garadagly.chat.server.config.ServerConfiguration;
import java.io.IOException;

public class StartServer {
    public static void main(String[] args) throws InterruptedException, IOException {
        ChatServer chatServer = new ChatServer(new ServerConfiguration());
        new Thread(chatServer).start();
        Thread.currentThread().join();
    }
}