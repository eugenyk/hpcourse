import protobuf.Message;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
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

    /**
     * Class for waiting messages.
     */
    private class Waiter implements Runnable {
        public void read() {

            try {
                Message.Msg msg = Message.Msg.parseDelimitedFrom(Channels.newInputStream(socketChannel));
                if (msg != null) {
                    System.out.println();
                    System.out.print(msg.getSender());
                    System.out.print(new StringBuffer(" [").append(msg.getDateTime()).append("] :"));
                    System.out.println(new StringBuffer(" ").append(msg.getText()));
                }
            } catch (IOException e) {
                // Disconnect.
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
        }

        public void run() {
           while (true) {
               read();
           }
        }
    }

    public ChatClient(String name) {
        this.name = name;
    }

    /**
     * Start client
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

    /**
     * Send message
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
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        System.out.print("Please, introduce yourself: ");

        BufferedReader consoleReader = new BufferedReader(
                new InputStreamReader(System.in));
        try {
            String name = consoleReader.readLine();
            ChatClient client = new ChatClient(name);
            if (!client.start()) {
                System.out.println("It's impossible to set connection!");
                return;
            }

            while (true) {
                System.out.print("You : ");
                String message = consoleReader.readLine();
                if (message != null) {
                    client.send(message);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
