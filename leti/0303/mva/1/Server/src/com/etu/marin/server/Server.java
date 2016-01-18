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
import java.util.concurrent.Executors;


public class Server implements Runnable {

    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private String host;
    private int port;
    private int threadsNum;
    private int buffer;
    private AsynchronousServerSocketChannel asynchronousServerSocketChannel;
    private final Set<AsynchronousSocketChannel> socketChannels = Collections.newSetFromMap(new ConcurrentHashMap<>());


    public Server(String host, int port, int threadsNum, int buffer) {
        this.host = host;
        this.port = port;
        this.threadsNum = threadsNum;
        this.buffer = buffer;

    }

    public synchronized void broadcast(byte[] msg) {
        for (AsynchronousSocketChannel socket : socketChannels) {
            socket.write(ByteBuffer.wrap(msg));
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
                        socketChannels.add(socketChannel);
                        readMessage(socketChannel);

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

    public void readMessage(AsynchronousSocketChannel socketChannel) {

        ByteBuffer buffer = ByteBuffer.allocate(this.buffer);
        ByteArrayOutputStream data = new ByteArrayOutputStream();

        socketChannel.read(buffer, data, new CompletionHandler<Integer, ByteArrayOutputStream>() {
            @Override
            public void completed(Integer result, ByteArrayOutputStream byteArrayOutputStream) {
                int length;
                try {
                    if (result == -1) {
                        socketChannel.close();
                        logger.info("Disconnect {}", socketChannel.getRemoteAddress());
                        socketChannels.remove(socketChannel);
                        return;
                    }

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

                    socketChannel.read(buffer, data, this);

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void failed(Throwable e, ByteArrayOutputStream byteArrayOutputStream) {
                try {
                    logger.error("Disconnect {}", socketChannel.getRemoteAddress());
                } catch (IOException e1) {
                    e1.printStackTrace();
                } finally {
                    socketChannels.remove(socketChannel);
                }
            }
        });
    }
}