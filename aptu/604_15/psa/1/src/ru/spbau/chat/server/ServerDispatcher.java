package ru.spbau.chat.server;

import ru.spbau.chat.commons.Reader;

import java.nio.channels.AsynchronousSocketChannel;
import java.util.List;

public interface ServerDispatcher extends Reader.Dispatcher {

    void onNewClient(final AsynchronousSocketChannel channel);

    void onCompletedCommand(final AsynchronousSocketChannel channel, final List<String> result);
}
