package com.niochat.common;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.Future;
import java.util.logging.Logger;

public class Client {
    private final static Logger log = Logger.getLogger(Client.class.getName());
    private AsynchronousSocketChannel socketChannel;
    private CallbackInterface callback;

    public Client(AsynchronousSocketChannel socketCh, Processor requestProcessor, CallbackInterface callback) {
        this.callback = callback;
        this.socketChannel = socketCh;
        ByteBuffer bf = Util.getTemporaryDirectBuffer(Constants.BUFFER_SIZE_IN_BYTES);
        socketChannel.read(bf, this, new CompletionHandler<Integer, Client>() {
            @Override
            public void completed(Integer result, Client attachment) {
                log.info("Read data from socket: " + result.intValue());
                if (result.intValue() == -1) {
                    try {
                        socketChannel.close();
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }
                    log.info("Client closed");
                    socketChannel = null; // Write null to mark that client is not active
                    return;
                }
                requestProcessor.process(bf, attachment);
                bf.clear();
                socketChannel.read(bf, attachment, this);
            }

            @Override
            public void failed(Throwable exc, Client attachment) {
                exc.printStackTrace();
                try {
                    socketChannel.close();
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
                log.info("Failed to read data from socket: " + exc.getMessage());
                socketChannel = null; // Write null to mark that client is not active
            }
        });
    }

    @Override
    public boolean equals(Object o) {
        if (o == null) return false;
        if (!(o instanceof Client)) return false;
        if (o == this) return true;
        if (((Client) o).socketChannel == null) return false;
        if (((Client) o).socketChannel.equals(socketChannel))
            return true;
        return false;
    }

    public boolean isActive() {
        return socketChannel != null;
    }

    public void close() {
        try {
            callback.processDisconnect();
            socketChannel.close();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        socketChannel = null;
    }

    public void write(byte[] msg) {
        try {
            Future<Integer> writeMessageFuture = socketChannel.write(ByteBuffer.wrap(msg));
            while (!writeMessageFuture.isDone()) ;
        } catch (Exception ex) {
            System.out.println(msg.length);
            for (int i = 0; i < msg.length; i++) {
                System.out.print (msg[i] + "  ");
            }
            System.out.println();
        }
    }
}
