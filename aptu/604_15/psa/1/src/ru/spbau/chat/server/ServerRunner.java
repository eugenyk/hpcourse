package ru.spbau.chat.server;

import java.io.IOException;

public class ServerRunner {

    public static void main(final String[] args) throws IOException {
        if (args.length != 2) {
            System.err.println("Invalid number of arguments");

            return;
        }

        final int port = Integer.parseInt(args[0]);
        final int threads = Integer.parseInt(args[1]);

        if (threads < 2) {
            System.err.println("Invalid number of threads");

            return;
        }

        new Server(port, threads).run();
    }
}
