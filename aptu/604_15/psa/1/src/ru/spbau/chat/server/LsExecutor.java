package ru.spbau.chat.server;

import java.io.File;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Arrays;
import java.util.List;

class LsExecutor implements Runnable {

    private final ServerDispatcher dispatcher;
    private final AsynchronousSocketChannel channel;

    public LsExecutor(final ServerDispatcher dispatcher, final AsynchronousSocketChannel channel) {
        this.dispatcher = dispatcher;
        this.channel = channel;
    }

    @Override
    public void run() {
        final List<String> result = Arrays.asList(new File(".").list());

        dispatcher.onCompletedCommand(channel, result);
    }
}
