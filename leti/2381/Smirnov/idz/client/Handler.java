import com.google.protobuf.InvalidProtocolBufferException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;

public class Handler implements CompletionHandler<Integer, Void> {

    private ByteBuffer receivedData;

    private ByteArrayOutputStream messageStream;


    private AsynchronousSocketChannel client;


    public Handler(AsynchronousSocketChannel client) {
        this.client = client;
        this.messageStream = new ByteArrayOutputStream();

        receivedData = ByteBuffer.allocate(1024);
    }

    public void start() {
        receivedData.position(0);

        client.read(receivedData, null, this);
    }

    @Override
    public void completed(Integer result, Void attachment) {
        if (result == -1) {
            return;
        }

        receivedData.flip();

        try {
            messageStream.write(Arrays.copyOfRange(receivedData.array(), 0, receivedData.limit()));
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (messageStream.size() >= 4) {

            int length = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();

            if (messageStream.size() - 4 == length) {

                Message.ClientMessage msg = null;
                try {
                    msg = Message.ClientMessage.parseFrom(Arrays.copyOfRange(messageStream.toByteArray(), 4, length + 4));
                } catch (InvalidProtocolBufferException e) {
                    e.printStackTrace();
                }
                System.out.format("Received message from %s : %s\n", msg.getSender(), msg.getText());

                messageStream.reset();
            }
        }

        receivedData.clear();

        client.read(receivedData, null, this);
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}