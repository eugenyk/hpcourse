package server;
import server.proto.MessageBody;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Executor;

/**
 * Created by lightwave on 14.12.15.
 */
public class ConsoleExecutor implements Runnable {
    private ConcurrentLinkedQueue<QueueElement> requests;
    private boolean isActive = true;

    public ConsoleExecutor() {
        requests = new ConcurrentLinkedQueue<QueueElement>();
    }


    @Override
    public void run() {
        processQueue();
    }

    private void processQueue() {
        while(true) {
            if(!requests.isEmpty()) {
                processRequest(requests.poll());
            }
        }
    }

    private void processRequest(QueueElement request) {
        AsynchronousSocketChannel user = request.getClient();
        String line, text = "";
        try {
            Runtime runtime = Runtime.getRuntime();
            Process process = runtime.exec(request.getRequest());
            BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()));
            while((line = br.readLine()) != null) {
                System.out.println("Line:" + line);
                text += line;
            }
        } catch(Exception e) {
            System.out.println("Console command error: " + e.toString());
        }
        MessageBody.Message message = MessageBody.Message.newBuilder().setSender("Server").setText(text).build();
        Responder responder = new Responder(user, message.toByteArray());
        System.out.println("Message length: " + message.toByteArray().length);
        responder.start();
    }

    public void addRequest(QueueElement elem) {
        requests.add(elem);
    }

    public void finish() {

    }

    
}
