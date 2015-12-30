package ru.spbau.pavlyutchenko.chat;


import ru.spbau.pavlyutchenko.chat.commons.protocol.ChatProtocol;

import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static ru.spbau.pavlyutchenko.chat.ReadCompletionHandler.readMessage;

public class Server implements Closeable {

    private final ExecutorService executorService = Executors.newSingleThreadExecutor();
    private final AsynchronousChannelGroup channelGroup;
    private final AsynchronousServerSocketChannel serverSocketChannel;
    private final Set<AsynchronousSocketChannel> socketChannels = Collections.newSetFromMap(new ConcurrentHashMap<>());

    public Server(int port, int threads) throws IOException {
        channelGroup = AsynchronousChannelGroup.withFixedThreadPool(threads - 1, Executors.defaultThreadFactory());

        serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup);
        serverSocketChannel.setOption(StandardSocketOptions.SO_REUSEADDR, true);
        serverSocketChannel.bind(new InetSocketAddress("localhost", port));
    }

    public void run() {
        try {
            AsynchronousSocketChannel socketChannel = serverSocketChannel.accept().get();
            socketChannels.add(socketChannel);

            readMessage(socketChannel,
                    message -> {
                        switch (message.getType()) {
                            case MESSAGE:
                                respond(message, socketChannel);
                                break;
                            case COMMAND:
                                lsCommand(message, socketChannel);
                                break;
                        }
                    },
                    result -> socketChannels.remove(socketChannel));
        } catch (InterruptedException | ExecutionException e) {
            System.err.println(e.getMessage());
        }
    }

    private void respond(ChatProtocol.Message message, AsynchronousSocketChannel filter) {
        ByteBuffer buffer = messageToByteBuffer(message);
        for (AsynchronousSocketChannel socketChannel : socketChannels) {
            if (!socketChannel.equals(filter)) {
                socketChannel.write(buffer, socketChannel, new WriteCompletionHandler());
            }
        }
    }

    private void lsCommand(ChatProtocol.Message message, AsynchronousSocketChannel socketChannel) {
        if (message.getTextCount() == 1 && message.getText(0).equals("command ls")) {
            executorService.submit(() -> {
                List<String> result = Arrays.asList(new File(".").list());

                ChatProtocol.Message msg = ChatProtocol.Message.newBuilder()
                        .setType(ChatProtocol.Message.Type.COMMAND)
                        .addAllText(result)
                        .build();
                socketChannel.write(messageToByteBuffer(msg), socketChannel, new WriteCompletionHandler());
            });
        }
    }

    private static ByteBuffer messageToByteBuffer(ChatProtocol.Message message) {
        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            message.writeDelimitedTo(outputStream);
            return ByteBuffer.wrap(outputStream.toByteArray());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void close() throws IOException {
        serverSocketChannel.close();
        channelGroup.shutdownNow();
    }
}
