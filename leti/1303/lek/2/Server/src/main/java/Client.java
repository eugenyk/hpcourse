import protobuf.Message;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.Channels;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by Елена on 06.01.2017.
 */
public class Client implements Comparable {
    private static AtomicInteger idCounter = new AtomicInteger(0);
    private final int id;
    private final AsynchronousSocketChannel socketChannel;
    private final Server server;
    private Message.Msg message;

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
            System.out.println(limit);
            byte[] received = new byte[limit];
            buffer.get(received, 0, limit).clear();
            return received;
        }

        public void completed(Integer result, Message.Msg attachment) {
            if (result == -1) {
                server.disconnect(Client.this);
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
                    server.broadcast(attachment, id);
                } else {
                    // Disconnect.
                    server.disconnect(Client.this);
                }
            } catch (IOException e) {
                System.out.println("Big message");
            }

            socketChannel.read(buffer, message, this);
        }

        public void failed(Throwable exc, Message.Msg attachment) {
            server.disconnect(Client.this);
        }
    }

    /**
     * Start working with client
     *
     * @param server
     */
    public void start(Server server) {
        new ReadHandler();
    }

    /**
     * Send message to real corresponding client.
     *
     * @param msg
     */
    public void send(Message.Msg msg) {
        try {
            msg.writeDelimitedTo(Channels.newOutputStream(socketChannel));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public Client(final AsynchronousSocketChannel socketChannel, Server server) {
        this.socketChannel = socketChannel;
        id = idCounter.getAndIncrement();
        this.server = server;
    }

    public int getId() {
        return id;
    }

    @Override
    public int compareTo(Object o) {
        return id - ((Client) o).id;
    }
}
