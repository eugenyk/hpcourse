package ru.spbau.chat.client;

import ru.spbau.chat.commons.Reader;
import ru.spbau.chat.commons.Writer;
import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Collections;
import java.util.concurrent.ExecutionException;

public class Client {

    private static final String CLIENT_NAME = "CLIENT";

    private final InetSocketAddress address;

    public Client(final InetSocketAddress address) {
        this.address = address;
    }

    public void run() throws ExecutionException, InterruptedException, IOException {
        try (final AsynchronousSocketChannel channel = AsynchronousSocketChannel.open()) {
            channel.connect(address).get();

            processIncoming(channel);
            processOutcoming(channel);
        }
    }

    private void processIncoming(final AsynchronousSocketChannel channel) {
        final Reader.Dispatcher dispatcher = new ReaderDispatcherImpl();
        final ByteBuffer buffer = ByteBuffer.allocate(dispatcher.getDefaultBufferSize());

        channel.read(buffer, null, new Reader(dispatcher, channel, buffer));
    }

    private void processOutcoming(final AsynchronousSocketChannel channel) {
        final BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        String line;

        try {
            while ((line = reader.readLine()) != null) {
                final ChatProtocol.Message message = ChatProtocol.Message.newBuilder()
                        .setType(
                                line.equals("command ls")
                                        ? ChatProtocol.Message.Type.COMMAND
                                        : ChatProtocol.Message.Type.MESSAGE
                        )
                        .addText(line)
                        .setAuthor(CLIENT_NAME)
                        .build();

                Writer.sendMessage(message, Collections.singleton(channel), null);
            }
        } catch (final IOException e) {
            e.printStackTrace();
            System.err.println(e.getMessage());
        }
    }
}
