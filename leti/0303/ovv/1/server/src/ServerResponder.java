import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

public class ServerResponder implements CompletionHandler<Integer, Void> {

    private AsynchronousSocketChannel receiver;

    byte[] message;

    public ServerResponder(AsynchronousSocketChannel receiver, byte[] message) {
        this.receiver = receiver;
        this.message = message;
    }

    public void send() {
        ByteBuffer buffer = ByteBuffer.allocate(message.length);

        buffer.put(message);
        buffer.position(0);

        try {
            receiver.write(buffer, null, this);
        } catch (Exception e) {
            send();
        }
    }

    @Override
    public void completed(Integer result, Void attachment) {

    }

    @Override
    public void failed(Throwable e, Void attachment) {
        e.printStackTrace();
    }

}
