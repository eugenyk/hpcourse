import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.Executors;
import java.util.logging.Logger;

/**
 * Created by Елена on 06.01.2017.
 */
public class Server {

    private int threadsNum = Runtime.getRuntime().availableProcessors();
    private final Config config = new Config();
    private final Logger logger = Logger.getLogger(this.getClass().getName());
    AsynchronousServerSocketChannel socketChannel = null;
    //ConcurrentSkipListSet<E>

    private class ResultConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Void> {

        public void completed(AsynchronousSocketChannel socket, Void server) {
            try {
                logger.info(new StringBuffer("Accepted a connection from ").append(socket.getRemoteAddress()).toString());
                socketChannel.accept(null, this);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void failed(Throwable e, Void attach) {
            logger.severe("Failed to accept a connection");
        }
    }

    public Server() {
    }

    public Server(int threadsNum) {
        if (threadsNum > 0) {
            this.threadsNum = threadsNum;
        }
    }

    public void start() {
        try {
            AsynchronousChannelGroup group =
                    AsynchronousChannelGroup.withFixedThreadPool(threadsNum, Executors.defaultThreadFactory());
            socketChannel = AsynchronousServerSocketChannel.open(group).
                                bind(new InetSocketAddress(config.getHost(), config.getPort()));
            socketChannel.accept(null, new ResultConnectionHandler());
        } catch (IOException e) {
            logger.severe("Couldn't start server");
        }
        logger.info(new StringBuffer("Server started at : \n\t host: ").
                            append(config.getHost()).
                            append("\n\t port: ").append(config.getPort()).
                            append("\n\t threads: ").append(threadsNum). toString());
    }

    public static void main(String[] args) {
        int threadsNumber = 0;
        if(args.length > 0) {
            threadsNumber = Integer.parseInt(args[0]);
        }
        new Server(threadsNumber).start();
    }
}
