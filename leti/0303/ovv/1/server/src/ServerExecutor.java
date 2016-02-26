import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ServerExecutor implements Runnable {

    private ConcurrentLinkedQueue<ServerExecutorRequest> requests = new ConcurrentLinkedQueue<ServerExecutorRequest>();

    @Override
    public void run() {
        while (true) {
            if(!requests.isEmpty()) {
                processRequest(requests.poll());
            }
        }
    }

    private void processRequest(ServerExecutorRequest request) {
        ServerClient user = request.getClient();

        Message.ClientMessage message = Message.ClientMessage.newBuilder().setSender("server").setText(executeRequest(request.getRequest())).build();

        user.responder.addMessage(ByteBuffer.allocate(4 + message.toByteArray().length).putInt(message.toByteArray().length).put(message.toByteArray()).array());
    }

    private String executeRequest(String request) {
        String line, text = "";

        try {
            Runtime runtime = Runtime.getRuntime();

            Process process = runtime.exec(request);

            BufferedReader buffer = new BufferedReader(new InputStreamReader(process.getInputStream()));

            while((line = buffer.readLine()) != null) {
                text += line + '\n';
            }

        } catch(Exception e) {
            return "Execute request error: " + e.toString() + "\n";
        }

        return text;
    }

    public void addRequest(ServerExecutorRequest request) {
        requests.add(request);
    }

}
