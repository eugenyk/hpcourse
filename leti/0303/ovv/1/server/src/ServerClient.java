import java.nio.channels.AsynchronousSocketChannel;

public class ServerClient {

    public final AsynchronousSocketChannel connection;
    public ServerResponder responder;

    public ServerClient(AsynchronousSocketChannel connection) {
        this.connection = connection;
        this.responder = new ServerResponder(this);
    }

}
