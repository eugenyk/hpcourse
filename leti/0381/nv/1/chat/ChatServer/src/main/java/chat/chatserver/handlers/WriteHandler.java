package chat.chatserver.handlers;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import org.apache.log4j.Logger;


public class WriteHandler implements CompletionHandler<Integer, ByteBuffer> {

    private final Logger logger = Logger.getLogger(WriteHandler.class);
    private final AsynchronousSocketChannel channel;
    
    public WriteHandler(AsynchronousSocketChannel channel) {
        this.channel = channel;
    }
    
    @Override
    public void completed(Integer result, ByteBuffer buffer) {
        if (buffer.hasRemaining()) {
            channel.write(buffer, buffer, this);
        }
    }

    @Override
    public void failed(Throwable ex, ByteBuffer attachment) {
        logger.error("Unable to write message!", ex);
    }
    
}
