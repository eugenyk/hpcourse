package client.core;

import client.api.Client;
import client.api.ClientListener;
import client.proto.MessageBody;
import com.google.protobuf.InvalidProtocolBufferException;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.LinkedBlockingDeque;


public class ChatClient implements Client {
    private static final Logger LOG = Logger.getLogger(ChatClient.class);
    public static final int CAPACITY = 65536;
    public static final int LENGTH = 4;
    private AsynchronousSocketChannel channel;
    private List<ClientListener> listeners;
    private BlockingDeque<byte[]> blocks;
    private ByteBuffer buffer;
    private int messageLength;
    private int appendix;
    private String clientName = "unknown";
    private String host = "127.0.0.1";
    private int port = 10000;

    public ChatClient() {
        buffer = ByteBuffer.allocate(CAPACITY);
        listeners = new ArrayList<>();
        blocks = new LinkedBlockingDeque<>();
    }

    public String getClientName() {
        return clientName;
    }

    public void setClientName(String clientName) {
        this.clientName = clientName;
    }

    public void setHost(String host) {
        this.host = host;
    }

    public void setPort(int port) {
        this.port = port;
    }

    @Override
    public AsynchronousSocketChannel getChannel() {
        return channel;
    }

    @Override
    public void connect() throws IOException {
        LOG.info(">>Connecting...");
        channel = AsynchronousSocketChannel.open();
        channel.connect(new InetSocketAddress(host, port), null, new CompletionHandler<Void, Void>() {
            @Override
            public void completed(Void result, Void attachment) {
                LOG.info("Connection complete");
                listeners.forEach(ClientListener::connectionComplete);
                read();
            }

            @Override
            public void failed(Throwable exc, Void attachment) {
                LOG.error("Connection failed");
            }
        });
    }

    @Override
    public void disconnect() throws IOException {
        channel.close();
        LOG.info("Disconnected");
        listeners.forEach(ClientListener::disconnectComplete);
    }

    @Override
    public void read() {
        buffer.position(0);
        channel.read(buffer, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result, Void attachment) {
                try {
                    if (result == -1) {
                        channel.close();
                        disconnect();
                    } else {
                        handleRead(result);
                    }
                } catch (Throwable ex) {
                    LOG.error("I/O operation failed", ex);
                }
            }

            @Override
            public void failed(Throwable ex, Void attachment) {
                try {
                    disconnect();
                } catch (IOException ignored) {}
            }
        });
    }

    @Override
    public void write(byte[] message) {
        ByteBuffer buffer = ByteBuffer.allocate(LENGTH + message.length);
        buffer.putInt(message.length);
        buffer.put(message);
        buffer.position(0);
        channel.write(buffer, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result, Void attachment) {
                LOG.info("Message write");
            }

            @Override
            public void failed(Throwable exc, Void attachment) {
                LOG.error("Failed write message");
            }
        });
    }

    public void registerListener(ClientListener listener) {
        listeners.add(listener);
    }

    public void unregisteredListener(ClientListener listener) {
        listeners.remove(listener);
    }

    private void handleRead(int result) {
        Optional<MessageBody.Message> message = formMessage(result);
        if (message.isPresent()) {
            listeners.forEach(listener -> listener.newMessage(message.get()));
        }
        read();
    }

    public Optional<MessageBody.Message> formMessage(int result) {
        buffer.position(0);
        if (blocks.size() == 0) {
            messageLength = appendix = buffer.getInt();
            result -= LENGTH;
        }

        byte[] block = new byte[result];
        buffer.get(block);
        blocks.add(block);
        appendix -= result;

        if (appendix == 0) {
            byte[] message = new byte[messageLength];
            int position = 0;
            while(!blocks.isEmpty()) {
                byte[] b = blocks.poll();
                int blockLength = b.length;
                System.arraycopy(b, 0, message, position, blockLength);
                position += blockLength;
            }

            try {
                return Optional.of(MessageBody.Message.parseFrom(message));
            } catch (InvalidProtocolBufferException e) {
                LOG.error("Failed parse message", e);
            }
        }

        return Optional.empty();
    }
}
