package com.github.oxaoo.chat.server;

import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 12.01.2017
 */
public class Server {
    private static final Logger LOG = LoggerFactory.getLogger(Server.class);

    private final int poolSize;

    private Set<AsynchronousSocketChannel> clients = new CopyOnWriteArraySet<>();
//    private Set<AsynchronousSocketChannel> clients = new ConcurrentSkipListSet<>();

    public Server(int poolSize) {
        this.poolSize = poolSize;
    }

    public void start(String host, int port) throws IOException {
        ExecutorService threadPool = Executors.newFixedThreadPool(this.poolSize);
        AsynchronousChannelGroup group = AsynchronousChannelGroup.withThreadPool(threadPool);
        AsynchronousServerSocketChannel listener = AsynchronousServerSocketChannel.open(group);
        InetSocketAddress listenAddress = new InetSocketAddress(host, port);

        listener.bind(listenAddress);

        AcceptConnectionHandler acceptCompletionHandler = new AcceptConnectionHandler(listener, this);
        listener.accept(null, acceptCompletionHandler);
        LOG.info("Start server on [{}:{}]", host, port);
    }

    public void addClient(AsynchronousSocketChannel client) {
        this.clients.add(client);
    }

    public void removeClient(AsynchronousSocketChannel client) {
        this.clients.remove(client);
    }

    public void broadcastMessage(AsynchronousSocketChannel sender, Message.ChatMessage message) {
        for (AsynchronousSocketChannel client : this.clients) {
//            if (client != sender) {
            if (client != null) {
                byte[] msgByte = message.toByteArray();
                ByteBuffer buffer = ByteBuffer.wrap(msgByte);
                client.write(buffer, null, new WriteMockHandler());
            }
        }
    }
}
