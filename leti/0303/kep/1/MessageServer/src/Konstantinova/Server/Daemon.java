package Konstantinova.Server;

import server.proto.MessageOuterClass;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Created by User on 20.12.2015.
 */
public class Daemon implements Runnable {
    ConcurrentLinkedQueue<QueueElem> queue;

    public Daemon() {
        this.queue = new ConcurrentLinkedQueue<QueueElem>();
    }

    @Override
    public void run() {
        while (true){
            if(!queue.isEmpty()){
                processCommand(queue.poll());
            }
        }
    }

    public void addQueueElem (QueueElem elem){
        queue.add(elem);
    }

    private void processCommand(QueueElem elem){
        String line, text = "";
        try {
            Runtime runtime = Runtime.getRuntime();
            Process process = runtime.exec(elem.getCommand());
            BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()));
            while((line = br.readLine()) != null) {
                System.out.println("Line:" + line);
                text += line;
            }
        } catch(Exception e) {
            System.out.println("Console command error: " + e.toString());
        }
        MessageOuterClass.Message message = MessageOuterClass.Message.newBuilder().setSender("Server").setText(text).build();
        AsynchronousSocketChannel author = elem.author;
        ArrayList<AsynchronousSocketChannel> list = new ArrayList<AsynchronousSocketChannel>();
        list.add(elem.getAuthor());
        MessageSender sender = new MessageSender(message.toByteArray(), list, null);
        sender.sendMessage();
    }
}
