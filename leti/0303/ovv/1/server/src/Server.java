import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executors;

public class Server implements CompletionHandler<AsynchronousSocketChannel, Void> {

    private InetSocketAddress address;
    private int numberOfThreads;

    private ServerExecutor executor = new ServerExecutor();

    private AsynchronousChannelGroup channelGroup;
    private AsynchronousServerSocketChannel serverSocketChannel;
    private CopyOnWriteArrayList<ServerClient> clients;

    public Server(InetSocketAddress address, int numberOfThreads) {
        this.address = address;
        this.numberOfThreads = numberOfThreads;

        Thread daemon = new Thread(executor);

        daemon.setDaemon(true);

        daemon.start();

        clients = new CopyOnWriteArrayList<ServerClient>();

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
        ServerClient tempClient = new ServerClient(client);

        try {
            String clientAddress = client.getRemoteAddress().toString();

            System.out.format("New connection with client address %s established.\n", clientAddress);

            clients.add(tempClient);
        } catch(Exception e) {
            e.printStackTrace();
        }

        serverSocketChannel.accept(null, this);

        ServerReceiver receiver = new ServerReceiver(clients, tempClient, executor);

        receiver.start();
    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
