import java.net.InetSocketAddress;

public class Server {

    private InetSocketAddress address;
    private int numberOfThreads;

    public Server(InetSocketAddress address, int numberOfThreads) {
        this.address = address;
        this.numberOfThreads = numberOfThreads;
    }

    public void start() {
        System.out.format("Server was started on %s with %d threads.", this.address.toString(), this.numberOfThreads);
    }

}
