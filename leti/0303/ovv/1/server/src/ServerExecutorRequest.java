import java.nio.channels.AsynchronousSocketChannel;

public class ServerExecutorRequest {

    private AsynchronousSocketChannel client;
    private String request;

    public ServerExecutorRequest(AsynchronousSocketChannel client, String request) {
        this.client = client;
        this.request = request;
    }

    public AsynchronousSocketChannel getClient() {
        return client;
    }

    public String getRequest() {
        return request;
    }

}
