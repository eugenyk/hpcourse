import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

public class ServerReceiver implements CompletionHandler<Integer, Void> {

    private ByteBuffer receivedData;

    private CopyOnWriteArrayList<AsynchronousSocketChannel> connections;

    private AsynchronousSocketChannel client;

    private ServerExecutor executor;

    public ServerReceiver(CopyOnWriteArrayList<AsynchronousSocketChannel> connections, AsynchronousSocketChannel client, ServerExecutor executor) {
        this.connections = connections;
        this.client = client;
        this.executor = executor;

        receivedData = ByteBuffer.allocate(4096);
    }

    public void start() {
        receivedData.position(0);

        client.read(receivedData, null, this);
    }

    @Override
    public void completed(Integer result, Void attachment) {
        if (result == -1) {
            removeClientFromConnections();
        } else {
            String message = new String(receivedData.array(), 0, receivedData.position());

            processMessage(message);

            start();
        }
    }

    private void processMessage(String message) {
        String clientAddress;

        try {
            clientAddress = client.getRemoteAddress().toString();

            System.out.format("Received message from %s: %s\n", clientAddress, message);

            if (message.matches("^/c [\\s\\S]+")) {
                executor.addRequest(new ServerExecutorRequest(client, message.substring(3)));
            } else {
                broadcastMessageForAllConnections((clientAddress + ": " + message) .getBytes());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void removeClientFromConnections() {
        try {
            connections.remove(client.getRemoteAddress().toString());

            System.out.format("Client with address %s was disconnected.\n", client.getRemoteAddress().toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void broadcastMessageForAllConnections(byte[] message) {
        Iterator<AsynchronousSocketChannel> iterator = connections.iterator();

        while (iterator.hasNext()) {
            AsynchronousSocketChannel connection = iterator.next();

            if(connection != client && connection.isOpen()) {
                ServerResponder responder = new ServerResponder(connection, message);

                responder.send();
            }
        }
    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
