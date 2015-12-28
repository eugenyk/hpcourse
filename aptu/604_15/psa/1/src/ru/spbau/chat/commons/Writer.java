package ru.spbau.chat.commons;

import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Collection;

public final class Writer {

    public static void sendMessage(final ChatProtocol.Message message,
                                   final Collection<AsynchronousSocketChannel> receivers,
                                   final AsynchronousSocketChannel excludedChannel) {
        final ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try {
            message.writeDelimitedTo(outputStream);
        } catch (final IOException e) {
            e.printStackTrace();
            System.err.println(e.getMessage());

            return;
        }

        final ByteBuffer buffer = ByteBuffer.wrap(outputStream.toByteArray());

        for (final AsynchronousSocketChannel receiver : receivers) {
            if (receiver != excludedChannel) {
                receiver.write(buffer);
            }
        }
    }
}
