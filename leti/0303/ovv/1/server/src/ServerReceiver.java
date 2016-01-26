import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Iterator;
import java.util.Map;

public class ServerReceiver implements CompletionHandler<Integer, Void> {

    private ByteBuffer recievedData;

    private Map<String, AsynchronousSocketChannel> connections;

    private AsynchronousSocketChannel client;

    public ServerReceiver(Map<String, AsynchronousSocketChannel> connections, AsynchronousSocketChannel client) {
        this.connections = connections;
        this.client = client;

        recievedData = ByteBuffer.allocate(8192);
    }

    public void start() {
        recievedData.position(0);

        client.read(recievedData, null, this);
    }

    @Override
    public void completed(Integer result, Void attachment) {
        if (result == -1) {
            removeClientFromConnections();
        } else {
            try {
                System.out.format("Received message from %s: %s\n", client.getRemoteAddress().toString(), new String(recievedData.array(), 0, recievedData.position()));

                broadcastMessageForAllConnections(recievedData.array());
            } catch (Exception e) {
                e.printStackTrace();
            }

            start();
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
        Iterator<AsynchronousSocketChannel> iterator = connections.values().iterator();

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
