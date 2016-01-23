package com.etu.marin.server;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by Vladislav on 23.01.2016.
 */
public class ClientHandler {

    private final AsynchronousSocketChannel socketChannel;
    final ConcurrentLinkedQueue<ByteBuffer> queue = new ConcurrentLinkedQueue<>();
    final AtomicBoolean sending = new AtomicBoolean();

    public ClientHandler(final AsynchronousSocketChannel socketChannel) {
        this.socketChannel= socketChannel;
    }

    public AsynchronousSocketChannel getSocketChannel(){
        return socketChannel;
    }

    public void write(byte[] msg)
    {
        queue.offer(ByteBuffer.wrap(msg));

        if (sending.compareAndSet(false, true)) {
            socketChannel.write(queue.poll(), null, new CompletionHandler<Integer, Void>() {
                @Override
                public void completed(Integer result, Void attachment) {
                    ByteBuffer next = queue.poll();
                    if(next != null){
                        socketChannel.write(next, null, this);
                    }else {
                        sending.set(false);
                    }

                }

                @Override
                public void failed(Throwable exc, Void attachment) {
                    sending.set(false);
                }
            });
        }
    }
}
