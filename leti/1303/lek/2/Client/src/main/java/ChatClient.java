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
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by Елена on 06.01.2017.
 */
public class ChatClient {
    private static AtomicInteger idCounter = new AtomicInteger(0);
    private final Config config = new Config();
    private final String name;
    private final int id;

    private AsynchronousSocketChannel socketChannel;

    public ChatClient(String name) {
        this.name = name;
        id = idCounter.getAndIncrement();
    }

    public void start() {

        try {
            socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddress = new InetSocketAddress(config.getHost(), config.getPort());
            Future<Void> result = socketChannel.connect(serverAddress);
            result.get();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void send(String message) {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
        Message.Msg msg = Message.Msg.newBuilder()
                .setText(message.trim())
                .setSender(new StringBuffer(name).append("_").append(id).toString())
                .setData(LocalDateTime.now().format(formatter))
                .build();
        try {
            msg.writeDelimitedTo(Channels.newOutputStream(socketChannel));
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
            client.start();

            while (true) {
                System.out.print("You : ");
                String message = consoleReader.readLine();
                client.send(message);

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
