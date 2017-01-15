package com.github.oxaoo.chat.client;

import com.github.oxaoo.chat.common.proto.Message;
import com.google.protobuf.InvalidProtocolBufferException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 15.01.2017
 */
public class MessageIncomingHandler implements CompletionHandler<Integer, Void> {
    private static final Logger LOG = LoggerFactory.getLogger(MessageIncomingHandler.class);

    private AsynchronousSocketChannel socketChannel;
    private ByteBuffer inputBuffer;

    public MessageIncomingHandler(AsynchronousSocketChannel socketChannel, ByteBuffer inputBuffer) {
        this.socketChannel = socketChannel;
        this.inputBuffer = inputBuffer;
    }

    @Override
    public void completed(Integer numBytes, Void attachment) {
        LOG.info("Read the input message");

        if (numBytes < 1) {
            try {
                LOG.debug("Server closed the connection: {}", this.socketChannel.getRemoteAddress().toString());
            } catch (IOException e) {
                LOG.error("Error while get server address connection. Cause: {}", e.toString());
            }
        } else {
            byte[] data = new byte[numBytes];
            this.inputBuffer.rewind();
            this.inputBuffer.get(data);

            try {
                Message.ChatMessage message = Message.ChatMessage.parseFrom(data);
                LOG.info("Got message from sever: {}", message.toString().replaceAll("\n", "; "));
            } catch (InvalidProtocolBufferException e) {
                LOG.error("Error while parse protobuf message. Cause: {}", e.toString());
            }

            this.inputBuffer.clear();
            this.socketChannel.read(this.inputBuffer, null, this);
        }
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }

}
