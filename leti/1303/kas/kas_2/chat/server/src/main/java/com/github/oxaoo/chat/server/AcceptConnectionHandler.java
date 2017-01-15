package com.github.oxaoo.chat.server;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 15.01.2017
 */
public class AcceptConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Void> {
    private static final Logger LOG = LoggerFactory.getLogger(AcceptConnectionHandler.class);

    private AsynchronousServerSocketChannel listener;
    private Server server;

    public AcceptConnectionHandler(AsynchronousServerSocketChannel listener, Server server) {
        this.listener = listener;
        this.server = server;
    }

    @Override
    public void completed(AsynchronousSocketChannel socketChannel, Void attachment) {
        try {
            LOG.info("Accept the connection from {}", socketChannel.getRemoteAddress().toString());
        } catch (IOException e) {
            LOG.error("Error while get address connection. Cause: {}", e.toString());
        }
        this.server.addClient(socketChannel); //add client to address list

        listener.accept(attachment, this);

        ByteBuffer inputBuffer = ByteBuffer.allocate(1024);
        ReadMessageHandler readMessageHandler = new ReadMessageHandler(socketChannel, inputBuffer, this.server);
        socketChannel.read(inputBuffer, null, readMessageHandler);
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}
