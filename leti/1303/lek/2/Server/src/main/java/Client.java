import protobuf.Message;

import java.io.IOException;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.Channels;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by Елена on 06.01.2017.
 */
public class Client implements Comparable {
    private static AtomicInteger idCounter = new AtomicInteger(0);
    private final int id;
    private final AsynchronousSocketChannel socketChannel;

    /**
     * Read message
     * @return message
     */
    public Message.Msg read() {

        try {
            Message.Msg msg = Message.Msg.parseDelimitedFrom(Channels.newInputStream(socketChannel));
            return msg;
        } catch (IOException e) {
            return null;
        }

    }

    /**
     * Start working with client
     * @param server
     */
    public void start(Server server) {
        while (true) {
            Message.Msg msg = read();
            if (msg != null) {
                server.broadcast(msg, id);
            } else {
                // Disconnect.
                server.disconnect(this);
            }
        }
    }

    /**
     * Send message to real corresponding client.
     * @param msg
     */
    public void send(Message.Msg msg) {
        try {
            msg.writeDelimitedTo(Channels.newOutputStream(socketChannel));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public Client(final AsynchronousSocketChannel socketChannel) {
        this.socketChannel = socketChannel;
        id = idCounter.getAndIncrement();
    }

    public int getId() {
        return id;
    }

    @Override
    public int compareTo(Object o) {
        return id - ((Client) o).id;
    }
}
