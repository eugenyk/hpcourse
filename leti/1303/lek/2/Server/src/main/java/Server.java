import protobuf.Message;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

/**
 * Created by Елена on 06.01.2017.
 */
public class Server {

    private int threadsNum = Runtime.getRuntime().availableProcessors();
    private final Config config = new Config();
    private final Logger logger = Logger.getLogger(this.getClass().getName());
    AsynchronousServerSocketChannel socketChannel = null;
    ConcurrentSkipListSet<Client> clients = new ConcurrentSkipListSet<>();

    public void disconnect(Client client) {
        clients.remove(client);
    }

    private class ResultConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Void> {

        public void completed(AsynchronousSocketChannel socket, Void server) {
            try {
                logger.info(new StringBuffer("Accepted a connection from ")
                        .append(socket.getRemoteAddress()).toString());
                socketChannel.accept(null, this);
                Client client = new Client(socket, Server.this);
                clients.add(client);
                client.start(Server.this);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void failed(Throwable e, Void attach) {
            logger.severe("Failed to accept a connection");
        }
    }

    /**
     * Broadcast message to all clients
     *
     * @param msg
     * @param id  client id from which message was gotten.
     */
    public void broadcast(Message.Msg msg, int id) {
        logger.info(new StringBuffer("Send message from client ").append(id).toString());
        for (Client client : clients) {
            if (id != client.getId()) {
                client.send(msg);
            }
        }
    }

    public Server(int threadsNum) {
        if (threadsNum > 0) {
            this.threadsNum = threadsNum;
        }
    }

    /**
     * Start server.
     */
    public void start() {
        try {
            AsynchronousChannelGroup group =
                    AsynchronousChannelGroup.withCachedThreadPool(
                            Executors.newCachedThreadPool(), threadsNum);
            socketChannel = AsynchronousServerSocketChannel.open(group).
                    bind(new InetSocketAddress(config.getHost(), config.getPort()));
            socketChannel.accept(null, new ResultConnectionHandler());
            logger.info(new StringBuffer("Server started at : \n\t host: ").
                    append(config.getHost()).
                    append("\n\t port: ").append(config.getPort()).
                    append("\n\t threads: ").append(threadsNum).toString());
            logger.info("Terminating ");
            group.awaitTermination(Long.MAX_VALUE, TimeUnit.SECONDS);

        } catch (IOException e) {
            logger.severe("Couldn't start server");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

    public static void main(String[] args) {
        int threadsNumber = 0;
        if (args.length > 0) {
            threadsNumber = Integer.parseInt(args[0]);
        }
        new Server(threadsNumber).start();
    }
}
