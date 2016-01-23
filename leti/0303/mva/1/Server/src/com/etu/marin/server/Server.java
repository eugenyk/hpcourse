package com.etu.marin.server;

import common.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.Arrays;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;


public class Server implements Runnable {

    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private String host;
    private int port;
    private int threadsNum;
    private int buffer;
    private AsynchronousServerSocketChannel asynchronousServerSocketChannel;
    private final Set<ClientHandler> socketChannels = Collections.newSetFromMap(new ConcurrentHashMap<>());



    public Server(String host, int port, int threadsNum, int buffer) {
        this.host = host;
        this.port = port;
        this.threadsNum = threadsNum;
        this.buffer = buffer;

    }

    public void broadcast(byte[] msg) {
        for (ClientHandler socket : socketChannels) {
            socket.write(msg);
        }
    }

    @Override
    public void run() {
        try {
            AsynchronousChannelGroup threadGroup =
                    AsynchronousChannelGroup.withFixedThreadPool(threadsNum - 1, Executors.defaultThreadFactory());
            asynchronousServerSocketChannel = AsynchronousServerSocketChannel.open(threadGroup);
            asynchronousServerSocketChannel.bind(new InetSocketAddress(host, port));
            logger.info("Server is listening {}:{}", host, port);
            asynchronousServerSocketChannel.accept(null, new CompletionHandler<AsynchronousSocketChannel, Void>() {

                @Override
                public void completed(AsynchronousSocketChannel socketChannel, Void server) {
                    try {
                        asynchronousServerSocketChannel.accept(null, this);
                        logger.info("Connect from {}", socketChannel.getRemoteAddress());
                        ClientHandler clientHandler = new ClientHandler(socketChannel);
                        socketChannels.add(clientHandler);
                        readMessage(clientHandler);

                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void failed(Throwable e, Void attach) {
                    logger.error("Connecting failed");
                }
            });
        } catch (IOException e) {
            logger.error("Server start fail");
            e.printStackTrace();
        }
    }

    public void readMessage(ClientHandler clientHandler) {

        ByteBuffer buffer = ByteBuffer.allocate(this.buffer);
        AsynchronousSocketChannel socketChannel = clientHandler.getSocketChannel();

        socketChannel.read(buffer, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result,  Void attachment) {
                int length;
                try {
                    if (result == -1) {

                        logger.info("Disconnect {}", socketChannel.getRemoteAddress());
                        socketChannels.remove(clientHandler);
                        socketChannel.close();
                        return;
                    }
                    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
                    buffer.flip();
                    int limit = buffer.limit();
                    byte[] msgBytes = new byte[limit];
                    buffer.get(msgBytes, 0, limit).clear();

                    byteArrayOutputStream.write(msgBytes);


                    if (byteArrayOutputStream.size() >= 4) {
                        length = ByteBuffer.wrap(
                                byteArrayOutputStream.toByteArray(), 0, 4).getInt();

                        if (byteArrayOutputStream.size() - 4 == length) {
                            byte[] message = Arrays.copyOfRange(byteArrayOutputStream.toByteArray(), 0, length + 4);

                            Message.Msg msg = Message.Msg.parseFrom(Arrays.copyOfRange(message, 4, length + 4));

                            logger.info("Message {}:{}", msg.getSender(), msg.getText());

                            byteArrayOutputStream.reset();

                            if (msg.getText().startsWith("/c")) {
                                (new Thread(new ProcessCommand(socketChannel, msg.getText()))).start();
                            } else {
                                broadcast(message);
                            }
                        }
                    }

                    socketChannel.read(buffer, null, this);

                } catch (IOException e) {
                    socketChannels.remove(clientHandler);
                    logger.error("Disconnect");
                    e.printStackTrace();
                }
            }

            @Override
            public void failed(Throwable e, Void attachment) {
                try {
                    logger.error("Disconnect {}", socketChannel.getRemoteAddress());
                } catch (IOException e1) {
                    e1.printStackTrace();
                } finally {
                    socketChannels.remove(clientHandler);
                }
            }
        });
    }
}