import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.Future;

/**
 * Created by Елена on 06.01.2017.
 */
public class ChatClient {
    private final Config config = new Config();

    public ChatClient() {
    }

    public void start() {

        try {
            AsynchronousSocketChannel socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddress = new InetSocketAddress(config.getHost(), config.getPort());
            Future<Void> result = socketChannel.connect(serverAddress);
            result.get();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        new ChatClient().start();
        BufferedReader consoleReader = new BufferedReader(
                new InputStreamReader(System.in));
        while (true) {
            try {
                String msg = consoleReader.readLine();
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.print(">");
        }
    }
}
