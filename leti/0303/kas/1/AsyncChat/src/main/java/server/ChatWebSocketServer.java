package server;

import org.apache.log4j.Logger;
import server.api.ChatHandler;
import server.api.ChatServer;
import server.util.NamedThreadFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.*;


public class ChatWebSocketServer implements ChatServer {
    private static final Logger LOG = Logger.getLogger(ChatWebSocketServer.class);
    private AsynchronousServerSocketChannel serverSocketChannel;
    private AsynchronousChannelGroup channelGroup;
    private InetSocketAddress serverAddress;
    private ChatHandler chatHandler;


    public ChatWebSocketServer(InetSocketAddress serverAddress, String serverName) throws IOException {
        this(serverAddress, serverName, Runtime.getRuntime().availableProcessors());
    }

    public ChatWebSocketServer(InetSocketAddress serverAddress, String serverName, int nThreads) throws IOException {
        this.serverAddress = serverAddress;
        chatHandler = new ChatWebSocketHandler();
        channelGroup = AsynchronousChannelGroup
                .withFixedThreadPool(nThreads, new NamedThreadFactory(serverName));
    }

    @Override
    public void startServer() throws IOException, InterruptedException {
        serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup).bind(serverAddress);

        Thread serverThread = new Thread(() -> {
            if (serverSocketChannel.isOpen()) {
                LOG.info(">>Starting server...");
                serverSocketChannel.accept(null, new CompletionHandler<AsynchronousSocketChannel, Object>() {
                    @Override
                    public void completed(final AsynchronousSocketChannel channel, Object attachment) {
                        if (serverSocketChannel.isOpen()) {
                            // accept the next connection
                            serverSocketChannel.accept(null, this);
                        }
                        // handle this connection
                        chatHandler.addWorker(channel);
                    }

                    @Override
                    public void failed(Throwable ex, Object attachment) {
                        if (serverSocketChannel.isOpen()) {
                            LOG.error("I/O operation failed", ex);
                        }
                    }
                });
                LOG.info(">>Server started.");
            }
        });
        serverThread.start();
        serverThread.join();

        Thread commandThread = new Thread(CommandThread.getInstance());
        commandThread.start();
        commandThread.join();
    }

    @Override
    public void stopServer() throws IOException {
        serverSocketChannel.close();
        channelGroup.shutdown();
        LOG.info(">>Server stopped.");
    }

    @Override
    public void restartServer() throws IOException, InterruptedException {
        stopServer();
        startServer();
    }

    @Override
    public ChatHandler getHandler() {
        return chatHandler;
    }
}
