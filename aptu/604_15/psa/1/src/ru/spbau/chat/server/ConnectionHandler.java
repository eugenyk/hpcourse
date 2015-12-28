package ru.spbau.chat.server;

import ru.spbau.chat.commons.Reader;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

class ConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Object> {

    private final ServerDispatcher dispatcher;
    private final AsynchronousServerSocketChannel channel;

    public ConnectionHandler(final ServerDispatcher dispatcher, final AsynchronousServerSocketChannel channel) {
        this.dispatcher = dispatcher;
        this.channel = channel;
    }

    @Override
    public void completed(final AsynchronousSocketChannel result, final Object attachment) {
        dispatcher.onNewClient(result);

        final ByteBuffer buffer = ByteBuffer.allocate(dispatcher.getDefaultBufferSize());
        result.read(buffer, null, new Reader(dispatcher, result, buffer));

        channel.accept(null, this);
    }

    @Override
    public void failed(final Throwable exc, final Object attachment) {
        exc.printStackTrace();
        System.err.println(exc.getMessage());
    }
}
