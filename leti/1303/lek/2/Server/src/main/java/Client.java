import com.google.protobuf.MessageLite;
import protobuf.Message;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.Channels;
import java.nio.channels.CompletionHandler;

/**
 * Created by Елена on 06.01.2017.
 */
public class Client {
    private final AsynchronousSocketChannel socketChannel;

    public void read() {

        try {
            Message.Msg msg = Message.Msg.parseDelimitedFrom(Channels.newInputStream(socketChannel));
            System.out.println(msg.getText());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public Client(final AsynchronousSocketChannel socketChannel) {
        this.socketChannel = socketChannel;
        while (true) {
            read();
        }
    }
}
