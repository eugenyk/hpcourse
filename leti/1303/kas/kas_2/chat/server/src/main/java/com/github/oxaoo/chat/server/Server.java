package com.github.oxaoo.chat.server;

import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Set;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 12.01.2017
 */
public class Server {
    private static final Logger LOG = LoggerFactory.getLogger(Server.class);

    private final ThreadPoolExecutor poolExecutor;
    private Selector selector;

    private boolean isRun;

    public Server(int poolSize) {
        int maxPullSize = poolSize * 2;
        this.poolExecutor = new ThreadPoolExecutor(poolSize, maxPullSize, 0, TimeUnit.SECONDS,
                new LinkedBlockingQueue<Runnable>());
    }

    public void start(String host, int port) throws IOException {
        final InetSocketAddress listenAddress = new InetSocketAddress(host, port);
        ServerSocketChannel serverChanel = ServerSocketChannel.open();
        serverChanel.configureBlocking(false); //non-blocking
        this.selector = Selector.open();

        serverChanel.socket().bind(listenAddress);
        serverChanel.register(this.selector, SelectionKey.OP_ACCEPT);

        LOG.info("Start server on [{}:{}]", host, port);
        this.isRun = true;
        this.listenEvent();
    }

    public void shutdown() {
        //todo implement...
        this.isRun = false;
    }

    //todo handle the throws
    private void listenEvent() throws IOException {
        while (this.isRun) {
            this.selector.select();
            Set<SelectionKey> keys = this.selector.selectedKeys();
            for (SelectionKey key : keys) {
                if (!key.isValid()) continue;
                if (key.isAcceptable()) this.acceptConnection(key);
                else if (key.isReadable()) this.readMessage(key);
            }
            keys.clear();
        }
    }

    private void acceptConnection(SelectionKey key) throws IOException {
        LOG.info("Accept the connection");
        ServerSocketChannel serverChannel = (ServerSocketChannel) key.channel();
        SocketChannel channel = serverChannel.accept();
        channel.configureBlocking(false);
        SocketAddress remoteAddress = channel.socket().getRemoteSocketAddress();
        LOG.debug("Connect from: {}", remoteAddress.toString());
        channel.register(this.selector, SelectionKey.OP_READ); //register for reading
    }

    private void readMessage(SelectionKey key) throws IOException {
        LOG.info("Read the input message");
        SocketChannel channel = (SocketChannel) key.channel();
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        int numRead = channel.read(buffer);

        if (numRead < 0) {
            SocketAddress remoteAddress = channel.socket().getRemoteSocketAddress();
            LOG.debug("Connection closed by client: {}", remoteAddress.toString());
            channel.close();
            key.cancel();
            return;
        }

        byte[] data = new byte[numRead];
        System.arraycopy(buffer.array(), 0, data, 0, numRead);
        Message.ChatMessage message = Message.ChatMessage.parseFrom(data);
        LOG.info("Got message: {}", message.toString().replaceAll("\n", "; "));
    }

    private void handle(Runnable task) {
        this.poolExecutor.execute(task);
    }
}
