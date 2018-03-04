import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executors;

public class Server implements CompletionHandler<AsynchronousSocketChannel, Void> {

    private InetSocketAddress address;
    private int numOfThreads;

    private AsynchronousChannelGroup channelGroup;
    private AsynchronousServerSocketChannel serverSocketChannel;
    private CopyOnWriteArrayList<AsynchronousSocketChannel> connections;
    private CopyOnWriteArrayList<String> users;

    public Server(InetSocketAddress address, int numOfThreads) {
    this.numOfThreads=numOfThreads;
    this.address=address;
    connections = new CopyOnWriteArrayList<AsynchronousSocketChannel>();
    users = new CopyOnWriteArrayList<String>();
    try {
        channelGroup = AsynchronousChannelGroup.withFixedThreadPool(numOfThreads, Executors.defaultThreadFactory());
        serverSocketChannel = AsynchronousServerSocketChannel.open(channelGroup).bind(address);
    } catch(Exception e) {
        e.printStackTrace();
    }
    }

    public void start() {

        System.out.format("Server was started on %s with %d threads.\n", this.address.toString(), this.numOfThreads);

        serverSocketChannel.accept(null, this);
    }
    @Override
    public void completed(AsynchronousSocketChannel client, Void attachment) {
        try {
            String clientAddress = client.getRemoteAddress().toString();

            System.out.format("New connection with client address %s established.\n", clientAddress);
            connections.add(client);
        } catch(Exception e) {
            e.printStackTrace();
        }

        serverSocketChannel.accept(null, this);
        Handler handler = new Handler(connections,users, client);

        handler.start();

    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
