package com.github.oxaoo.chat.client;

import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 14.01.2017
 */
public class Client {
    private static final Logger LOG = LoggerFactory.getLogger(Client.class);

    private final SocketChannel clientChannel;

    public Client(String serverHost, int serverPort) throws IOException {
        InetSocketAddress hostAddress = new InetSocketAddress(serverHost, serverPort);
        clientChannel = SocketChannel.open(hostAddress);
    }

    public void closeConnection() throws IOException {
        clientChannel.close();
    }

    public void sendMessage(String msg) throws IOException {
        byte[] msgByte = msg.getBytes();
        ByteBuffer buffer = ByteBuffer.wrap(msgByte);
        clientChannel.write(buffer);
        buffer.clear();
        LOG.info("Send the message: {}", msg);
    }

    public void sendMessage(Message.ChatMessage message) throws IOException {
        byte[] msgByte = message.toByteArray();
        ByteBuffer buffer = ByteBuffer.wrap(msgByte);
        clientChannel.write(buffer);
        buffer.clear();
        LOG.info("Send the message: {}", message.toString());
    }
}
