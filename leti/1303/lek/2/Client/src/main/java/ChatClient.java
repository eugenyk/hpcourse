import protobuf.Message;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.Channels;
import java.nio.channels.CompletionHandler;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.Future;

/**
 * Created by Елена on 06.01.2017.
 */
public class ChatClient {

    private final Config config = new Config();
    private final String name;
    private AsynchronousSocketChannel socketChannel;
    private final Client client;
    private Message.Msg message;

    /**
     * Class for waiting messages.
     */
    private class ReadHandler implements CompletionHandler<Integer, Message.Msg> {
        private ByteBuffer buffer;
        ByteArrayOutputStream data = new ByteArrayOutputStream();

        public ReadHandler() {
            buffer = ByteBuffer.allocate(2048);
            socketChannel.read(buffer, message, this);
        }

        private byte[] getBytes(ByteBuffer buffer) {
            buffer.flip();
            int limit = buffer.limit();
            byte[] received = new byte[limit];
            buffer.get(received, 0, limit).clear();
            return received;
        }

        public void completed(Integer result, Message.Msg attachment) {
            if (result == -1) {
                disconnect();
                client.connectionLost();
                return;
            }
            try {
                data.write(getBytes(buffer));
            } catch (IOException e) {
                e.printStackTrace();
            }
            ByteArrayInputStream input = new ByteArrayInputStream(data.toByteArray());

            try {
                attachment = Message.Msg.parseDelimitedFrom(input);
                data.reset();
                if (attachment != null) {
                    client.showMessage(attachment);
                }
            } catch (IOException e) {
                // Disconnect.
                disconnect();
                client.connectionLost();
            }

            socketChannel.read(buffer, message, this);
        }

        public void failed(Throwable exc, Message.Msg attachment) {
            disconnect();
            client.connectionLost();
        }
    }

    public ChatClient(String name, Client client) {
        this.name = name;
        this.client = client;
    }

    /**
     * Start client
     *
     * @return success flag
     */
    public boolean start() {

        try {
            socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddress = new InetSocketAddress(config.getHost(), config.getPort());
            Future<Void> result = socketChannel.connect(serverAddress);
            result.get();
            new ReadHandler();
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public void disconnect() {
        try {
            if (socketChannel.isOpen()) {
                socketChannel.shutdownOutput();
                socketChannel.close();
            }
        } catch (IOException e1) {
            e1.printStackTrace();
        }
    }

    /**
     * Send message
     *
     * @param message
     */
    public void send(String message) {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
        Message.Msg msg = Message.Msg.newBuilder()
                .setText(message.trim())
                .setSender(name)
                .setDateTime(LocalDateTime.now().format(formatter))
                .build();
        try {
            if (socketChannel.isOpen()) {
                msg.writeDelimitedTo(Channels.newOutputStream(socketChannel));
            } else {
                System.out.println("Server was shutted down!");
            }
        } catch (IOException e) {
            client.connectionLost();
            disconnect();
        }
    }
}
