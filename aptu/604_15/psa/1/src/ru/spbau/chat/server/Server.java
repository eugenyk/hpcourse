package ru.spbau.chat.server;

import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.ConsoleHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Server {

    public static final String SERVER_NAME = "SERVER";

    private static final Logger LOGGER = Logger.getLogger(Server.class.getCanonicalName());

    private final int port;
    private final int threads;

    private final Set<ClientHandler> clientHandlers;

    private final ExecutorService executorService;

    public Server(final int port, final int threads) {
        initLogger();

        this.port = port;
        this.threads = threads;

        clientHandlers = Collections.newSetFromMap(new ConcurrentHashMap<ClientHandler, Boolean>());

        executorService = Executors.newSingleThreadExecutor();
    }

    public void run() throws IOException {
        final AsynchronousChannelGroup channelGroup = AsynchronousChannelGroup
                .withFixedThreadPool(
                        threads - 1,
                        Executors.defaultThreadFactory()
                );

        try {
            run(channelGroup);
        } finally {
            channelGroup.shutdownNow();

            LOGGER.log(Level.INFO, "Channel group has been shutdown" );
        }
    }

    boolean onOutcomingOverload(final ClientHandler clientHandler) {
        log(clientHandler, "Outcoming overload. Messages may be lost", null, Level.WARNING);

        return false;
    }

    void onNewMessage(final ClientHandler clientHandler, final ChatProtocol.Message message) {
        switch (message.getType()) {
            case COMMAND:
                processNewCommand(clientHandler, message);
                break;
            case MESSAGE:
                processNewMessage(clientHandler, message);
                break;
            default:
                processUnexpectedMessageType(clientHandler);
        }
    }

    boolean onIncomingOverload(final ClientHandler clientHandler) {
        log(clientHandler, "Incoming overload. Client will be disconnected", null, Level.WARNING);

        disconnect(clientHandler);

        return false;
    }

    void onReadEnd(final ClientHandler clientHandler) {
        log(clientHandler, "Read end. Client will be removed", null, Level.INFO);

        clientHandlers.remove(clientHandler);
    }

    boolean onReadFailed(final ClientHandler clientHandler, final Throwable t) {
        log(clientHandler, "Read failed. Client will be disconnected", t, Level.WARNING);

        disconnect(clientHandler);

        return false;
    }

    boolean onWriteFailed(final ClientHandler clientHandler, final Throwable t) {
        log(clientHandler, "Write failed. Client will be disconnected", t, Level.WARNING);

        disconnect(clientHandler);

        return false;
    }

    void onSerializeFailed(final ClientHandler clientHandler, final ChatProtocol.Message message, final IOException e) {
        log(clientHandler, String.format("Serialize failed [author: %s]", message.getAuthor()), e, Level.WARNING);
    }

    private void initLogger() {
        final Handler consoleHandler = new ConsoleHandler();
        consoleHandler.setLevel(Level.ALL);

        LOGGER.addHandler(consoleHandler);
        LOGGER.setLevel(Level.ALL);
    }

    private void run(final AsynchronousChannelGroup channelGroup) throws IOException {
        try (
                AsynchronousServerSocketChannel channel = AsynchronousServerSocketChannel
                        .open(channelGroup)
                        .bind(new InetSocketAddress(port))
        ) {
            run(channel);
        } catch (final InterruptedException e) {
            LOGGER.log(Level.WARNING, "", e);
        } finally {
            LOGGER.log(Level.INFO, "Channel has been closed" );
        }
    }

    private void run(final AsynchronousServerSocketChannel channel) throws InterruptedException {
        while (true) {
            try {
                final ClientHandler clientHandler = new ClientHandler(this, channel.accept().get());

                clientHandlers.add(clientHandler);

                log(clientHandler, "New client", null, Level.INFO);
            } catch (final ExecutionException e) {
                LOGGER.log(Level.WARNING, "", e);
            }
        }
    }

    private void log(final ClientHandler clientHandler, final String message, final Throwable t, final Level level) {
        try {
            LOGGER.log(
                    level,
                    String.format("%s [address: %s]", message, clientHandler.getRemoteAddress().toString()),
                    t
            );
        } catch (final IOException e) {
            LOGGER.log(Level.WARNING, "", e);
        }
    }

    private void processNewCommand(final ClientHandler clientHandler, final ChatProtocol.Message message) {
        if (message.getTextCount() == 1 && message.getText(0).equals("command ls" )) {
            log(
                    clientHandler,
                    String.format("New ls [author: %s]", message.getAuthor()),
                    null,
                    Level.FINEST
            );

            executorService.submit(new LsExecutor(clientHandler));
        } else {
            log(
                    clientHandler,
                    String.format("Invalid command [command: %s, author: %s]", message.getText(0), message.getAuthor()),
                    null,
                    Level.FINE
            );

            final ChatProtocol.Message response = ChatProtocol.Message.newBuilder()
                    .setType(ChatProtocol.Message.Type.MESSAGE)
                    .addText("Invalid command" )
                    .setAuthor(SERVER_NAME)
                    .build();

            clientHandler.send(response);
        }
    }

    private void processNewMessage(final ClientHandler clientHandler, final ChatProtocol.Message message) {
        log(clientHandler, String.format("New message [author: %s]", message.getAuthor()), null, Level.FINEST);

        for (final ClientHandler handler : clientHandlers) {
            if (handler != clientHandler) {
                handler.send(message);
            }
        }
    }

    private void processUnexpectedMessageType(final ClientHandler clientHandler) {
        final ChatProtocol.Message response = ChatProtocol.Message.newBuilder()
                .setType(ChatProtocol.Message.Type.MESSAGE)
                .addText("Unexpected message type" )
                .setAuthor(SERVER_NAME)
                .build();

        clientHandler.send(response);
    }

    private void disconnect(final ClientHandler clientHandler) {
        clientHandlers.remove(clientHandler);

        try {
            clientHandler.close();

            log(clientHandler, "Client has been disconnected", null, Level.WARNING);
        } catch (final IOException e) {
            log(clientHandler, "Disconnecting problem occurred", e, Level.WARNING);
        }
    }
}
