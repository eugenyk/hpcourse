package com.niochat.client;

import com.google.protobuf.InvalidProtocolBufferException;
import com.niochat.common.CallbackInterface;
import com.niochat.common.Client;
import com.niochat.common.Constants;
import com.niochat.common.Processor;
import com.niochat.message.ProtoMessage;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

public class ClientMain implements Runnable{
    private final static Logger log = Logger.getLogger(ClientMain.class.getName());
    private String                          host;
    private int                             port;
    private String                          name;
    Client                                  client;
    CallbackInterface callback;

    public ClientMain(CallbackInterface callback) {
        this.callback = callback;
    }

    @Override
    public void run() {
        try {
            AsynchronousSocketChannel socketChannel = AsynchronousSocketChannel.open();
            log.info("Starting with:");
            log.info("Host=" + this.host);
            log.info("Port=" + this.port);


            SocketAddress serverAddr = new InetSocketAddress(host, port);
            System.out.println("Connecting");
            Future<Void> result = socketChannel.connect(serverAddr);
            result.get(5, TimeUnit.SECONDS);
            if (result.isCancelled()) {
                callback.processDisconnect();
            }
            log.info("Connected");
            client = new Client(socketChannel, new Processor() {
                @Override
                public void process(ByteBuffer bf, Client client) {
                    try {
                        bf.flip();
                        int limit = bf.limit();
                        byte[] received = new byte[limit];
                        bf.get(received);
                        ProtoMessage.Message msg = ProtoMessage.Message.parseFrom(received);
                        callback.processMessage(msg.getSender() + ": " + msg.getText() + "\n");
                    } catch (InvalidProtocolBufferException e) {
                        e.printStackTrace();
                    }
                }
            }, callback);
            callback.processConnect();
        } catch(Exception e) {
            System.out.println("Exception, server of port " +this.port+ " terminating. Stack trace:");
            e.printStackTrace();
        }
    }

    public void send(String message) {
        ProtoMessage.Message msg = ProtoMessage.Message.newBuilder()
                .setText(message.trim())
                .setSender(name)
                .build();
        client.write(msg.toByteArray());
    }

    public void disconnect() {
        callback.processDisconnect();
        client.close();
    }

    public void connect(String host, int port, String name) {
        if (client != null && client.isActive())
                client.close();

        this.host = host == null ? Constants.host : host;
        this.port = port < 100 ? Constants.port : port;
        this.name = name == null ? "NULL" : name;

        new Thread(this).run();
    }
}
