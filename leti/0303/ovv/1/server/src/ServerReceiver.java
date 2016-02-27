import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

public class ServerReceiver implements CompletionHandler<Integer, Void> {

    private ByteBuffer receivedData;

    private ByteArrayOutputStream messageStream;

    private CopyOnWriteArrayList<ServerClient> clients;

    private ServerClient client;

    private ServerExecutor executor;

    public ServerReceiver(CopyOnWriteArrayList<ServerClient> clients, ServerClient client, ServerExecutor executor) {
        this.clients = clients;
        this.client = client;
        this.executor = executor;
        this.messageStream = new ByteArrayOutputStream();

        receivedData = ByteBuffer.allocate(128);
    }

    public void start() {
        receivedData.position(0);

        client.connection.read(receivedData, null, this);
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

        while (messageStream.size() >= 4 && hasFullMessage()) {
            putMessage();
        }

        receivedData.clear();

        client.connection.read(receivedData, null, this);
    }

    private boolean hasFullMessage() {
        int length = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();

        return messageStream.size() - 4 >= length;
    }

    private void putMessage() {
        int length = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();

        if (messageStream.size() - 4 >= length) {

            processMessage(Arrays.copyOfRange(messageStream.toByteArray(), 0, length + 4), length);

            byte[] temp = Arrays.copyOfRange(messageStream.toByteArray(), length + 4, messageStream.size());

            messageStream.reset();

            try {
                messageStream.write(temp);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private void processMessage(byte[] message, int length) {
        Message.ClientMessage msg;

        try {
            msg = Message.ClientMessage.parseFrom(Arrays.copyOfRange(message, 4, length + 4));

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
            clients.remove(client);

            System.out.format("Client with address %s was disconnected.\n", client.connection.getRemoteAddress().toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void broadcastMessageForAllConnections(final byte[] message) {
        Iterator<ServerClient> iterator = clients.iterator();

        while (iterator.hasNext()) {
            final ServerClient connection = iterator.next();

            if(connection.connection != client.connection && connection.connection.isOpen()) {
                connection.responder.addMessage(message);
            }
        }
    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
