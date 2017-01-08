import protobuf.Message;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.Channels;
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
    private Thread waiter;
    private final Client client;

    /**
     * Class for waiting messages.
     */
    private class Waiter implements Runnable {
        public boolean read() {

            try {
                Message.Msg msg = Message.Msg.parseDelimitedFrom(Channels.newInputStream(socketChannel));
                if (msg != null) {
                    client.showMessage(msg);
                    return true;
                }
            } catch (IOException e) {
                // Disconnect.
                disconnect();
                client.connectionLost();
                return false;
            }
            return false;
        }

        public void run() {
            while (read()) {
            }
        }
    }

    public ChatClient(String name, Client client) {
        this.name = name;
        this.client = client;
    }

    /**
     * Start client
     *
     * @return succes flag
     */
    public boolean start() {

        try {
            socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddress = new InetSocketAddress(config.getHost(), config.getPort());
            Future<Void> result = socketChannel.connect(serverAddress);
            result.get();
            Waiter waiterTask = new Waiter();
            waiter = new Thread(waiterTask);
            waiter.start();
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
            waiter.interrupt();
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
