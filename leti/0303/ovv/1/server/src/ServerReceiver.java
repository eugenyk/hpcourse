import com.google.protobuf.InvalidProtocolBufferException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

public class ServerReceiver implements CompletionHandler<Integer, Void> {

    private ByteBuffer receivedData;

    private ByteArrayOutputStream messageStream;

    private CopyOnWriteArrayList<AsynchronousSocketChannel> connections;

    private AsynchronousSocketChannel client;

    private ServerExecutor executor;

    public ServerReceiver(CopyOnWriteArrayList<AsynchronousSocketChannel> connections, AsynchronousSocketChannel client, ServerExecutor executor) {
        this.connections = connections;
        this.client = client;
        this.executor = executor;
        this.messageStream = new ByteArrayOutputStream();

        receivedData = ByteBuffer.allocate(1024);
    }

    public void start() {
        receivedData.position(0);

        client.read(receivedData, null, this);
    }

    @Override
    public void completed(Integer result,  Void attachment) {
        if (result == -1) {
            removeClientFromConnections();
            return;
        }

        receivedData.flip();

        try {
            messageStream.write(Arrays.copyOfRange(receivedData.array(), 0, receivedData.limit()));
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (messageStream.size() >= 4) {

            int length = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();

            if (messageStream.size() - 4 == length) {

                processMessage(messageStream.toByteArray(), length);

                messageStream.reset();
            }
        }

        receivedData.clear();

        client.read(receivedData, null, this);
    }

    private void processMessage(byte[] message, int length) {
        Message.ClientMessage msg;

        try {
            msg = Message.ClientMessage.parseFrom(Arrays.copyOfRange(messageStream.toByteArray(), 4, length + 4));

            System.out.format("Received message from %s : %s\n", msg.getSender(), msg.getText());

            if (msg.getText().matches("^/c [\\s\\S]+")) {
                executor.addRequest(new ServerExecutorRequest(client, msg.getText().substring(3)));
            } else {
                broadcastMessageForAllConnections(message);
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
