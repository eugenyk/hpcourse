package ru.leti.chat.server;

import ru.leti.chat.utils.protoMessage.ProtoMessage;

import java.io.Closeable;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.logging.Level;
import java.util.logging.Logger;

import static ru.leti.chat.utils.handlersIO.CompletionHandlerInput.submitInputTask;
import static ru.leti.chat.utils.handlersIO.CompletionHandlerOutput.submitOutputTask;
import static ru.leti.chat.utils.protoMessage.ConvertMessage.receivingBufferFormMessage;

/**
 * @author Ivan Motovilov
 *
 * <p>Класс реализации сервера чата</p>
 */
public class ChatServer implements Closeable {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println(args.length);
            System.out.println("Need use - Server <port> <pool_size>");
            return;
        }

        try (ChatServer server = new ChatServer(Integer.parseInt(args[0]), Integer.parseInt(args[1]))) {

        } catch (IOException e) {
            log.log(Level.SEVERE, "Server is closed", e);
        }
    }

    private static Logger log = Logger.getLogger(ChatServer.class.getName());

    private final AsynchronousChannelGroup channelGroup;
    private final AsynchronousServerSocketChannel serverSocketChannel;
    private final ConcurrentHashMap<AsynchronousSocketChannel, Long> connections = new ConcurrentHashMap<>();

    private ChatServer(int port, int poolSize) throws IOException {
        log.info("Server running...");

        channelGroup = AsynchronousChannelGroup.withFixedThreadPool(poolSize - 1,
                Executors.defaultThreadFactory());

        //options SO_REUSEADDR - позволяет использовать порт с новым состоянием, даже если он занят
        serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup)
                .setOption(StandardSocketOptions.SO_REUSEADDR, true)
                .bind(new InetSocketAddress("localhost", port));

        while (true) {
            try {
                AsynchronousSocketChannel socketChannel = serverSocketChannel.accept().get();
                connections.put(socketChannel, System.currentTimeMillis());

                submitInputTask(socketChannel,
                        message -> sendAllConnections(message),
                        result -> connections.remove(socketChannel));
            } catch (InterruptedException | ExecutionException e) {
                log.log(Level.SEVERE, "Channel not open", e);
            }
        }
    }

    /**
     * <p>Рассылка сообщения всем клиентам</p>
     * @param message Объект proto-сообщения
     */
    private void sendAllConnections(ProtoMessage.ChatMessage message) {
        ByteBuffer buffer = receivingBufferFormMessage(message);
        connections.forEachKey(1,
                socketChannel -> submitOutputTask(socketChannel,
                        buffer, throwable -> log.log(Level.SEVERE, "Error sending message", throwable)));

    }

    @Override
    public void close() throws IOException {
        serverSocketChannel.close();
        channelGroup.shutdownNow();
    }
}
