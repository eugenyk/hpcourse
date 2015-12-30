package ru.spbau.pavlyutchenko.chat;


import ru.spbau.pavlyutchenko.chat.commons.protocol.ChatProtocol;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.function.Consumer;


public class ReadCompletionHandler implements CompletionHandler<Integer, AsynchronousSocketChannel> {

    private final ByteBuffer buffer;
    private final Consumer<ChatProtocol.Message> messageConsumer;
    private final Consumer<Integer> resultConsumer;

    private ReadCompletionHandler(ByteBuffer buffer,
                                  Consumer<ChatProtocol.Message> messageConsumer,
                                  Consumer<Integer> resultConsumer) {
        this.buffer = buffer;
        this.messageConsumer = messageConsumer;
        this.resultConsumer = resultConsumer;
    }

    public static void readMessage(AsynchronousSocketChannel socketChannel,
                                   Consumer<ChatProtocol.Message> messageConsumer,
                                   Consumer<Integer> resultConsumer) {
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        socketChannel.read(buffer,
                socketChannel,
                new ReadCompletionHandler(buffer, messageConsumer, resultConsumer));
    }

    @Override
    public void completed(Integer result, AsynchronousSocketChannel attachment) {
        if (result <= 0) {
            System.err.println("ReadCompletionHandler, error code: " + result);
        } else {
            try {
                ChatProtocol.Message message = ChatProtocol.Message.parseDelimitedFrom(new ByteArrayInputStream(buffer.array()));
                messageConsumer.accept(message);
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                readMessage(attachment, messageConsumer, resultConsumer);
            }
        }
    }

    @Override
    public void failed(Throwable exc, AsynchronousSocketChannel attachment) {
        System.err.println("Failed to accept a connection.");
        exc.printStackTrace();
    }

}