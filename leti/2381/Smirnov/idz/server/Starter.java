import java.net.InetSocketAddress;
import java.nio.ByteBuffer;

public class Starter {
    public static void main(String[] args)
    {

        int numOfThreads = Integer.parseInt(args[0]);
        String server_hostname = "localhost";
        int server_port = 10000;
        Server server = new Server(new InetSocketAddress(server_hostname, server_port), numOfThreads);
        server.start();
    }

}
