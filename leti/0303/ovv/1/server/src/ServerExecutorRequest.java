import java.nio.channels.AsynchronousSocketChannel;

public class ServerExecutorRequest {

    private AsynchronousSocketChannel client;
    private String request;
    private byte[] message;
    private String type;

    public ServerExecutorRequest(AsynchronousSocketChannel client, String request, byte[] message, String type) {
        this.client = client;
        this.request = request;
        this.message = message;
        this.type = type;
    }

    public AsynchronousSocketChannel getClient() {
        return client;
    }

    public String getRequest() {
        return request;
    }

    public byte[] getMessage() {
        return message;
    }

    public String getType() {
        return type;
    }

}
