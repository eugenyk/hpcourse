package ChatClient2;

import java.io.IOException;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.CompletionHandler;
import java.util.function.Consumer;


public abstract class CompletionHandlerIO implements CompletionHandler<Integer, AsynchronousSocketChannel> {

    protected final Consumer<Throwable> errorGeneration;

    protected CompletionHandlerIO(Consumer<Throwable> errorGeneration) {
        this.errorGeneration = errorGeneration;
    }

    protected abstract void onFulfilled(AsynchronousSocketChannel socketChannel) throws IOException;

    public void completed(Integer result, AsynchronousSocketChannel attachment) {
        try {
            //result - Число прочитанных байт (-1, если ничего не прочитано)
            if (result != 1) {
                onFulfilled(attachment);
            } else {
                failed(new ClosedChannelException(), attachment);
            }
        } catch (IOException e) {
            failed(e, attachment);
        }
    }

    public void failed(Throwable exc, AsynchronousSocketChannel attachment) {
        errorGeneration.accept(exc);
    }
}