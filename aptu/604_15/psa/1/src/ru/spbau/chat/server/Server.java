package ru.spbau.chat.server;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.util.concurrent.Executors;

public class Server {

    private final int port;
    private final int threads;

    public Server(final int port, final int threads) {
        this.port = port;
        this.threads = threads;
    }

    public void run() throws IOException {
        final AsynchronousChannelGroup channelGroup = AsynchronousChannelGroup
                .withFixedThreadPool(
                        threads - 1,
                        Executors.defaultThreadFactory()
                );

        final ServerDispatcher dispatcher = new ServerDispatcherImpl();

        try (
                final AsynchronousServerSocketChannel channel = AsynchronousServerSocketChannel
                        .open(channelGroup)
                        .bind(new InetSocketAddress(port))
        ) {
            channel.accept(null, new ConnectionHandler(dispatcher, channel));

            while (true) ;
        }
    }
}
