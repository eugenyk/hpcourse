package ru.spbau.chat.avesloguzova;


import com.google.protobuf.ProtocolStringList;
import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Logger;
import java.util.stream.Collectors;

import static ru.spbau.chat.commons.protocol.ChatProtocol.Message.parseDelimitedFrom;

public class Server implements Closeable {
    private static final int BUFFFER_CAPACITY = 1 << 10;
    private static final Logger LOG = Logger.getLogger(Server.class.getName());

    private final AsynchronousChannelGroup channelGroup;
    private final AsynchronousServerSocketChannel serverSocketChannel;
    private final Set<AsynchronousSocketChannel> channels = ConcurrentHashMap.newKeySet();
    private final ExecutorService commandExecutor = Executors.newSingleThreadExecutor();

    public Server(int messagePoolSize, int port) throws IOException {
        channelGroup = AsynchronousChannelGroup.withFixedThreadPool(messagePoolSize, Executors.defaultThreadFactory());
        serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup);
        serverSocketChannel.bind(new InetSocketAddress("localhost", port));
        LOG.config("Start server on port" + port);
    }

    public void accept() {
        try {
            AsynchronousSocketChannel channel = serverSocketChannel.accept().get();
            channels.add(channel);
            startReadingNewMessage(channel);
        } catch (InterruptedException | ExecutionException e) {
            e.printStackTrace();
        }
    }

    private void startReadingNewMessage(AsynchronousSocketChannel channel) {
        ByteBuffer sizeBuffer = ByteBuffer.allocate(BUFFFER_CAPACITY);
        channel.read(sizeBuffer, channel, new ReadMessageHandler(sizeBuffer
        ));
    }

    @Override
    public void close() throws IOException {
        serverSocketChannel.close();
        channelGroup.shutdownNow();
    }

    private void handleMessage(ChatProtocol.Message message, AsynchronousSocketChannel channel) {
        switch (message.getType()) {
            case COMMAND:
                handleCommand(message.getTextList(), channel);
                break;
            case MESSAGE:
                shareMessage(message, channel);
                break;
        }
    }

    private void handleCommand(ProtocolStringList strings, AsynchronousSocketChannel channel) {
        commandExecutor.submit((Runnable) () -> {
            String command = buildCommand(strings);
            List<String> result = ChatUtils.runCommand(command);
            writeCommandResult(result, channel);
        });


    }

    private void writeCommandResult(List<String> result, AsynchronousSocketChannel channel) {
        ChatProtocol.Message message = ChatProtocol.Message.newBuilder()
                .addAllText(result)
                .setType(ChatProtocol.Message.Type.COMMAND)
                .build();
        sendMessage(message, channel);
    }

    private void shareMessage(ChatProtocol.Message message, AsynchronousSocketChannel sourceChanel) {
        channels.forEach(destinationChanel -> {
            if (!destinationChanel.equals(sourceChanel)) {
                sendMessage(message, destinationChanel);
            }
        });
    }

    private void sendMessage(ChatProtocol.Message message, AsynchronousSocketChannel destinationChanel) {
        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            message.writeDelimitedTo(outputStream);
            ByteBuffer buffer = ByteBuffer.wrap(outputStream.toByteArray());
            destinationChanel.write(buffer, destinationChanel, new WriteMessageHandler(buffer));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static String buildCommand(ProtocolStringList strings) {

        return strings.stream().collect(Collectors.joining(System.lineSeparator()));
    }

    public static void main(String[] args) {

        try (Server server = new Server(Integer.parseInt(args[0]), 20053)) {
            while (!Thread.interrupted()) {
                server.accept();
            }
        } catch (IOException e) {
            LOG.severe("Server is closed. " + e.getMessage());
        } catch (NumberFormatException e) {
            System.out.println("usage: Server <pool_size>");
        }
    }

    private class ReadMessageHandler extends ChatMessageHandler {

        ReadMessageHandler(ByteBuffer buffer) {
            super(buffer);
        }

        @Override
        protected void handleResult(Integer result, AsynchronousSocketChannel channel) {
            try {
                ChatProtocol.Message message = parseDelimitedFrom(new ByteArrayInputStream(buffer.array()));
                handleMessage(message, channel);
                startReadingNewMessage(channel);
            } catch (IOException e) {
                ByteBuffer byteBuffer = ByteBuffer.allocate(BUFFFER_CAPACITY);
                channel.read(byteBuffer, channel, new BufferedReadMessageHandler(buffer, byteBuffer));
            }
        }
    }

    private class BufferedReadMessageHandler extends ReadMessageHandler {

        private ByteBuffer oldBuffer;

        BufferedReadMessageHandler(ByteBuffer oldBuffer, ByteBuffer buffer) {
            super(buffer);
            this.oldBuffer = oldBuffer;
        }

        @Override
        protected void handleResult(Integer result, AsynchronousSocketChannel channel) {
            buffer = ChatUtils.concatBuffers(oldBuffer, buffer);
            super.handleResult(result, channel);
        }

    }

    private class WriteMessageHandler extends ChatMessageHandler {

        WriteMessageHandler(ByteBuffer buffer) {
            super(buffer);
        }

        @Override
        protected void handleResult(Integer result, AsynchronousSocketChannel channel) {
            if (buffer.hasRemaining()) {
                channel.write(buffer, channel, this);
            }

        }
    }

    private abstract class ChatMessageHandler implements CompletionHandler<Integer, AsynchronousSocketChannel> {
        protected ByteBuffer buffer;

        ChatMessageHandler(ByteBuffer buffer) {
            this.buffer = buffer;
        }

        protected abstract void handleResult(Integer result, AsynchronousSocketChannel attachment);

        @Override
        public void completed(Integer result, AsynchronousSocketChannel channel) {
            if (result > 0) {
                handleResult(result, channel);
            } else {
                failed(new ClosedChannelException(), channel);
            }
        }

        @Override
        public void failed(Throwable throwable, AsynchronousSocketChannel channel) {
            channels.remove(channel);
            LOG.info("Some exception occurred while in I/O operation in chanel " + channel.toString() +
                    ". Maybe chanel was closed.");
        }
    }
}