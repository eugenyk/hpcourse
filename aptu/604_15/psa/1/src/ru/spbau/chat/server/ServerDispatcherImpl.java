package ru.spbau.chat.server;

import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.nio.channels.AsynchronousSocketChannel;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static ru.spbau.chat.commons.Writer.sendMessage;

class ServerDispatcherImpl implements ServerDispatcher {

    private static final int DEFAULT_BUFFER_SIZE = 4096;
    private static final String SERVER_NAME = "SERVER";

    private final Set<AsynchronousSocketChannel> clients;
    private final ExecutorService executorService;

    public ServerDispatcherImpl() {
        clients = Collections.newSetFromMap(new ConcurrentHashMap<AsynchronousSocketChannel, Boolean>());
        executorService = Executors.newSingleThreadExecutor();
    }

    @Override
    public int getDefaultBufferSize() {
        return DEFAULT_BUFFER_SIZE;
    }

    @Override
    public void onNewClient(final AsynchronousSocketChannel channel) {
        clients.add(channel);
    }

    @Override
    public void onNewMessage(final AsynchronousSocketChannel channel, final ChatProtocol.Message message) {
        switch (message.getType()) {
            case COMMAND:
                processNewCommand(channel, message);
                break;
            case MESSAGE:
                processNewMessage(channel, message);
                break;
            default:
                processUnexpectedMessageType(channel);
        }
    }

    @Override
    public void onCompletedCommand(final AsynchronousSocketChannel channel, final List<String> result) {
        final ChatProtocol.Message response = ChatProtocol.Message.newBuilder()
                .setType(ChatProtocol.Message.Type.MESSAGE)
                .addAllText(result)
                .setAuthor(SERVER_NAME)
                .build();

        sendMessage(response, Collections.singleton(channel), null);
    }

    private void processNewCommand(final AsynchronousSocketChannel channel, final ChatProtocol.Message message) {
        if (message.getTextCount() == 1 && message.getText(0).equals("command ls")) {
            executorService.submit(new LsExecutor(this, channel));
        } else {
            final ChatProtocol.Message response = ChatProtocol.Message.newBuilder()
                    .setType(ChatProtocol.Message.Type.MESSAGE)
                    .addText("Invalid command")
                    .setAuthor(SERVER_NAME)
                    .build();

            sendMessage(response, Collections.singleton(channel), null);
        }
    }

    private void processNewMessage(final AsynchronousSocketChannel channel, final ChatProtocol.Message message) {
        sendMessage(message, clients, channel);
    }

    private void processUnexpectedMessageType(final AsynchronousSocketChannel channel) {
        final ChatProtocol.Message response = ChatProtocol.Message.newBuilder()
                .setType(ChatProtocol.Message.Type.MESSAGE)
                .addText("Unexpected message type")
                .setAuthor(SERVER_NAME)
                .build();

        sendMessage(response, Collections.singleton(channel), null);
    }
}
