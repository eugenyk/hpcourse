package server;

import server.proto.MessageBody;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.List;
import java.util.Map;

/**
 * Created by lightwave on 19.12.15.
 */
public class Reciever implements CompletionHandler<Integer, Void> {

    private MessageBuffer buffer = null;
    private Map<String, AsynchronousSocketChannel> connections;
    private ConsoleExecutor executor;
    private AsynchronousSocketChannel client;
    ByteBuffer data;

    public Reciever(Map<String, AsynchronousSocketChannel> connections, AsynchronousSocketChannel client, ConsoleExecutor executor) {
        this.connections = connections;
        this.client = client;
        this.executor = executor;
        data = ByteBuffer.allocate(MessageServer.BUFFERSIZE);
        buffer = new MessageBuffer();
    }

    public Reciever(Map<String, AsynchronousSocketChannel> connections, AsynchronousSocketChannel client, ConsoleExecutor executor, MessageBuffer buffer) {
        this.connections = connections;
        this.client = client;
        this.executor = executor;
        data = ByteBuffer.allocate(MessageServer.BUFFERSIZE);
        this.buffer = buffer;
    }

    public void start() {
        client.read(data, null, this);
    }


    @Override
    public void completed(Integer result, Void aVoid) {
        System.out.println("Something has been recieved. Length:" + result);
        if(result == -1) {
            try {
                connections.remove(client.getRemoteAddress().toString());
            }
            catch (Exception e) {}
            return;
        }
        Reciever rec = new Reciever(connections, client, executor, buffer);


        MessageBody.Message message = null;
        try {
            if (buffer.isClean()) {
                message = tryReadNewMessage(result);
            } else {

                message = tryReadBufferiezedMessage(result);
            }
        } catch (Exception e) {
            buffer.clean();
            data.position(0);
            rec.start();
            System.out.println("error compl" + e.toString());
        }

        try {
            data.position(0);
            rec.start();
            if(message != null) {
                buffer.clean();
                System.out.println("Message has been recieved. Sender:" + message.getSender() +
                        ". Text: " + message.getText());

                if(!message.getText().substring(0,3).equals("/c ")) {
                    Responder responder = new Responder(connections, client, message.toByteArray());
                    responder.start();
                } else {
                    executor.addRequest( new QueueElement(client, message.getText().substring(3)));
                }
            }

        }
        catch (Exception e) {
            buffer.clean();
            data.position(0);
            System.out.println("error compl" + e.toString());
        }
    }

    protected MessageBody.Message tryReadNewMessage(Integer result) {
        byte[] arr = new byte[result - 4];

        data.position(0);
        int messageLength = data.getInt();
        if(messageLength > MessageServer.MAXMESSAGELENGTH )
            return  null;
        MessageBody.Message message = null;
        System.out.println("Message length is: \"" + messageLength + "\"");
        data.position(4);
        data.get(arr);
        try {
            if (messageLength <= result) {
                message = MessageBody.Message.parseFrom(arr);

            } else {
                buffer.create(arr, messageLength);
            }
        } catch (Exception e) {
            buffer.clean();
            System.out.println("New message error" + e.toString());
        }

        return message;
    }

    protected  MessageBody.Message tryReadBufferiezedMessage(Integer result) {
        byte[] arr = new byte[result];
        data.get(arr);
        MessageBody.Message message = null;
        try {
            buffer.append(arr);
            if(buffer.getCurrentLength() < buffer.getLength()) {

            } else {
                message = MessageBody.Message.parseFrom(buffer.makeMessage());
            }
        }
        catch (Exception e) {
            buffer.clean();
            System.out.println("Bufferiezed message error " + e.toString());
        }

        return message;
    }

    @Override
    public void failed(Throwable throwable, Void aVoid) {

    }

    public void setBuffer(MessageBuffer buffer) {
        this.buffer = buffer;
    }
}
