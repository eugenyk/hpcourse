import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

public class Server implements CompletionHandler<AsynchronousSocketChannel, Void> {

    private InetSocketAddress address;
    private int numberOfThreads;

    private ServerExecutor executor = new ServerExecutor();

    private AsynchronousChannelGroup channelGroup;
    private AsynchronousServerSocketChannel serverSocketChannel;
    private ConcurrentHashMap<String, AsynchronousSocketChannel> connections;

    public Server(InetSocketAddress address, int numberOfThreads) {
        this.address = address;
        this.numberOfThreads = numberOfThreads;

        Thread daemon = new Thread(executor);

        daemon.setDaemon(true);

        daemon.start();

        connections = new ConcurrentHashMap<String, AsynchronousSocketChannel>();

        initChanelGroup(address, numberOfThreads);
    }

    private void initChanelGroup(InetSocketAddress address, int numberOfThreads) {
        try {
            channelGroup = AsynchronousChannelGroup.withFixedThreadPool(numberOfThreads, Executors.defaultThreadFactory());
            serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup).bind(address);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public void start() {
        System.out.format("Server was started on %s with %d threads.\n", this.address.toString(), this.numberOfThreads);

        serverSocketChannel.accept(null, this);
    }

    @Override
    public void completed(AsynchronousSocketChannel client, Void attachment) {
        try {
            String clientAddress = client.getRemoteAddress().toString();

            System.out.format("New connection with client address %s established.\n", clientAddress);

            connections.put(clientAddress, client);
        } catch(Exception e) {
            e.printStackTrace();
        }

        serverSocketChannel.accept(null, this);

        ServerReceiver receiver = new ServerReceiver(connections, client, executor);

        receiver.start();
    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
