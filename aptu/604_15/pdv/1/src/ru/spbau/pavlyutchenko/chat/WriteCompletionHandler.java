package ru.spbau.pavlyutchenko.chat;


import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

public class WriteCompletionHandler implements CompletionHandler<Integer, AsynchronousSocketChannel> {
    @Override
    public void completed(Integer result, AsynchronousSocketChannel attachment) {
        if (result <= 0) {
            System.err.println("WriteCompletionHandler, error code: " + result);
        }
    }

    @Override
    public void failed(Throwable exc, AsynchronousSocketChannel attachment) {
        System.err.println("Failed to accept a connection.");
        exc.printStackTrace();
    }
}
