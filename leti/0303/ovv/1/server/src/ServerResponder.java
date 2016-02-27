import java.nio.ByteBuffer;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Semaphore;

public class ServerResponder {

    private ServerClient receiver;

    private ConcurrentLinkedQueue<byte[]> messages = new ConcurrentLinkedQueue<byte[]>();

    private final Semaphore semaphore;

    public ServerResponder(ServerClient receiver) {
        this.receiver = receiver;
        this.semaphore = new Semaphore(1);
    }

    public void addMessage(byte[] message) {

        if (message == null) {
            return;
        }

        try {
            semaphore.acquire();

            ByteBuffer msg = ByteBuffer.allocate(message.length).put(message);

            msg.position(0);

            receiver.connection.write(msg, null, new CompletionHandler<Integer, Integer>() {
                @Override
                public synchronized void completed(Integer result, Integer attachment) {
                    semaphore.release();
                    addMessage(messages.poll());
                }

                @Override
                public void failed(Throwable exc, Integer attachment) {
                    semaphore.release();
                }
            });
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

}
