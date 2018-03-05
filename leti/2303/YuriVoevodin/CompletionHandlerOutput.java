package ChatServer2;

import ChatServer2.Message;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.function.Consumer;

import static ChatServer2.ConvertMessage.receivingBufferFormMessage;
public class CompletionHandlerOutput extends CompletionHandlerIO {
    private CompletionHandlerOutput(Consumer<Throwable> errorGeneration) {
        super(errorGeneration);
    }

    protected void onFulfilled(AsynchronousSocketChannel socketChannel) throws IOException {
    }
    public static void submitOutputTask(AsynchronousSocketChannel socketChannel,
                                        ByteBuffer byteBuffer,
                                        Consumer<Throwable> errorGeneration) {
        socketChannel.write(byteBuffer, socketChannel, new CompletionHandlerOutput(errorGeneration));
    }
    public static void submitOutputTask(AsynchronousSocketChannel socketChannel,
                                        Message.Msg chatMessage,
                                        Consumer<Throwable> errorGeneration) throws IOException {
        submitOutputTask(socketChannel, receivingBufferFormMessage(chatMessage), errorGeneration);

    }
}
