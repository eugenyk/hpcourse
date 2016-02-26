package Konstantinova.Server;

import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by User on 13.12.2015.
 */
public class Server implements CompletionHandler<AsynchronousSocketChannel, Void> {
    AsynchronousChannelGroup channelGroup;
    CopyOnWriteArrayList<AsynchronousSocketChannel> connections;
    AsynchronousServerSocketChannel serverSocketChannel;
    Daemon daemon;

    public Server(AsynchronousChannelGroup channelGroup, Daemon d) {
        this.channelGroup = channelGroup;
        try {
            serverSocketChannel = AsynchronousServerSocketChannel.open().bind(new InetSocketAddress("192.168.1.104", 6061));
        } catch (Exception e){
            System.out.println(e.toString());
        }
        connections = new CopyOnWriteArrayList<AsynchronousSocketChannel>();
        daemon = d;
    }

    public void start(){
        serverSocketChannel.accept(null, this);
    }

    @Override
    public void completed(AsynchronousSocketChannel result, Void attachment) {
        connections.add(result);
        try {
            System.out.println(result.getRemoteAddress());
        } catch (Exception e) {}
        MessageReader newMsg = new MessageReader(result, connections, daemon);
        newMsg.start();
        serverSocketChannel.accept(null, this);
        //result.read()
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}
