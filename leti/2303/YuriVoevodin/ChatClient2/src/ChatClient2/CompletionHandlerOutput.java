package ChatClient2;

import ChatClient2.Message;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.function.Consumer;

import static ChatClient2.ConvertMessage.receivingBufferFormMessage;


public class CompletionHandlerOutput extends ChatClient2.CompletionHandlerIO {
    private CompletionHandlerOutput(Consumer<Throwable> errorGeneration) {
        super(errorGeneration);
    }

    @Override
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
