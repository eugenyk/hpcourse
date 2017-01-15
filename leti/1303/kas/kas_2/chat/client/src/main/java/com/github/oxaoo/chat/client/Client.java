package com.github.oxaoo.chat.client;

import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 14.01.2017
 */
public class Client {
    private static final Logger LOG = LoggerFactory.getLogger(Client.class);

    private final AsynchronousSocketChannel clientChannel;

    public Client(String serverHost, int serverPort) throws IOException, ExecutionException, InterruptedException {
        InetSocketAddress hostAddress = new InetSocketAddress(serverHost, serverPort);
        this.clientChannel = AsynchronousSocketChannel.open();
        this.clientChannel.connect(hostAddress);
        LOG.info("Client connect to server");
        this.listenIncoming();
    }

    public void closeConnection() throws IOException {
        this.clientChannel.shutdownOutput();
    }

    public void sendMessage(Message.ChatMessage message) throws IOException {
        byte[] msgByte = message.toByteArray();
        ByteBuffer buffer = ByteBuffer.wrap(msgByte);
        clientChannel.write(buffer);
        buffer.clear();
        LOG.info("Send the message: {}", message.toString().replaceAll("\n", "; "));
    }

    private void listenIncoming() {
        LOG.info("Begin listen incoming message from server...");
        ByteBuffer inputBuffer = ByteBuffer.allocate(1024);
        MessageIncomingHandler messageIncomingHandler = new MessageIncomingHandler(this.clientChannel, inputBuffer);
        this.clientChannel.read(inputBuffer, null, messageIncomingHandler);
    }
}
