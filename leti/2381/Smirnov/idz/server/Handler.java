import com.google.protobuf.InvalidProtocolBufferException;
import com.sun.xml.internal.ws.encoding.MtomCodec;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

public class Handler implements CompletionHandler<Integer, Void> {

    private ByteBuffer receivedData;

    private ByteArrayOutputStream messageStream;

    private CopyOnWriteArrayList<AsynchronousSocketChannel> connections;
    private CopyOnWriteArrayList<String> users;

    private AsynchronousSocketChannel client;


    public Handler(CopyOnWriteArrayList<AsynchronousSocketChannel> connections,CopyOnWriteArrayList<String> users, AsynchronousSocketChannel client) {
        this.connections = connections;
        this.users=users;
        this.client = client;
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
            if(msg.hasData()) {
                if(msg.getData().equals("auth")) {
                    if (!users.contains(msg.getSender())) {
                        users.add(msg.getSender());
                        Message.ClientMessage.Builder b = Message.ClientMessage.newBuilder();
                        b.setSender("server");
                        b.setText("new user " + msg.getSender());
                        Message.ClientMessage msg_server = b.build();
                        int len = msg_server.toByteArray().length;
                        ByteBuffer bb = ByteBuffer.allocate(4 + len);
                        bb.putInt(len);
                        bb.put(msg_server.toByteArray());
                        broadcastMessageForAllConnections(bb.array());
                    }
                    else {
                        Message.ClientMessage.Builder b = Message.ClientMessage.newBuilder();
                        b.setSender("server");
                        b.setText("such username already exists");
                        Message.ClientMessage msg_server = b.build();
                        int len = msg_server.toByteArray().length;
                        ByteBuffer bb = ByteBuffer.allocate(4 + len);
                        bb.putInt(len);
                        bb.put(msg_server.toByteArray());
                        bb.position(0);
                        client.write(bb);
                    }
                }
                if(msg.getData().equals("q")) {
                    users.remove(msg.getSender());
                    Message.ClientMessage.Builder b = Message.ClientMessage.newBuilder();
                    b.setSender("server");
                    b.setText("left user " + msg.getSender());
                    Message.ClientMessage msg_server = b.build();
                    int len = msg_server.toByteArray().length;
                    ByteBuffer bb = ByteBuffer.allocate(4 + len);
                    bb.putInt(len);
                    bb.put(msg_server.toByteArray());
                    broadcastMessageForAllConnections(bb.array());
                }
            } else {
                System.out.format("Received message from %s : %s\n", msg.getSender(), msg.getText());

                if (users.contains(msg.getSender())) {
                    broadcastMessageForAllConnections(message);
                }
                else{
                    System.out.format("this user is not authorized\n");
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void removeClientFromConnections() {
        try {
            connections.remove(client);

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
                ByteBuffer buffer = ByteBuffer.allocate(message.length);

                buffer.put(message);
                buffer.position(0);

                connection.write(buffer);
            }
        }
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}
