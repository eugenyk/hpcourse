package Konstantinova.Server;

import server.proto.MessageOuterClass;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by User on 13.12.2015.
 */
public class MessageReader implements CompletionHandler<Integer, Void>{

    private ByteBuffer msg;
    AsynchronousSocketChannel clientListener;
    CopyOnWriteArrayList<AsynchronousSocketChannel> connections;
    Daemon daemon;
    MessageParser parser;

    public MessageReader(AsynchronousSocketChannel clientListener, CopyOnWriteArrayList<AsynchronousSocketChannel> connections, Daemon d){
        this.clientListener = clientListener;
        this.connections = connections;
        try {
            msg = ByteBuffer.allocate(1500);
        } catch (Exception e){}
        daemon = d;
        parser = new MessageParser();
    }

    public void start(){
        clientListener.read(msg, null, this);
    }

    @Override
    public void completed(Integer result, Void attachment) {

        if (result == -1){
            connections.remove(clientListener);
            return;
        }
        System.out.println("Message length = " + result);
        List<MessageOuterClass.Message> messageList;
        messageList = parser.parseBuffer(msg, result);

        Iterator<MessageOuterClass.Message> iter = messageList.iterator();
        while(iter.hasNext()) {
            MessageOuterClass.Message message = iter.next();
            System.out.println(message.getSender());
            System.out.println(message.getText());

            if (message.getText().substring(0, 3).equals("/c ")) {
                daemon.addQueueElem(new QueueElem(message.getText().substring(3), clientListener));
                return;
            }

            MessageSender sender = new MessageSender(message.toByteArray(), connections, clientListener);
            sender.sendMessage();
        }
        msg.position(0);
        clientListener.read(msg, null, this);
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}
