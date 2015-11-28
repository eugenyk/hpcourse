package ru.spbau.chat.client;

import ru.spbau.chat.commons.Reader;
import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.nio.channels.AsynchronousSocketChannel;

class ReaderDispatcherImpl implements Reader.Dispatcher {

    private static final int DEFAULT_BUFFER_SIZE = 4096;

    @Override
    public int getDefaultBufferSize() {
        return DEFAULT_BUFFER_SIZE;
    }

    @Override
    public void onNewMessage(final AsynchronousSocketChannel channel, final ChatProtocol.Message message) {
        System.out.print(message.getAuthor());
        System.out.println(": ");

        for (final String s : message.getTextList()) {
            System.out.println(s);
        }
    }
}
