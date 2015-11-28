package ru.spbau.chat.client;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.concurrent.ExecutionException;

public class ClientRunner {

    public static void main(final String[] args) throws InterruptedException, ExecutionException, IOException {
        if (args.length != 2) {
            System.err.println("Invalid number of arguments");

            return;
        }

        final int port = Integer.parseInt(args[1]);

        new Client(new InetSocketAddress(args[0], port)).run();
    }
}
