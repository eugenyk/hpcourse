package ru.spbau.pavlyutchenko.chat;


import java.io.IOException;

public class ServerRunner {
    public static void main(String[] args) throws IllegalAccessException {
        if (args.length < 2) {
            throw new IllegalAccessException("Invalid number of arguments (port and number of threads are required.");
        }

        try (Server server = new Server(Integer.parseInt(args[0]), Integer.parseInt(args[1]))) {
            while (true) {
                server.run();
            }
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }
}
