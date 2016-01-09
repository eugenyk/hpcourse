package edu.etu.server;

import java.io.IOException;

public class StartServer {
    public static void main(String[] args) throws InterruptedException, IOException {
        ChatServer chatServer = new ChatServer(new ServerConfiguration());
        new Thread(chatServer).start();
        Thread.currentThread().join();
    }
}