package com.github.parfenovvs.websocketchatserver.api;

import com.github.parfenovvs.websocketchatserver.entity.Message;
import com.google.gson.Gson;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Controller("handler")
public class Handler extends WebSocketServer {
    private final ClientManager clientManager;
    private Gson gson;
    private ExecutorService executorService;

    @Autowired
    public Handler(InetSocketAddress socketAddress, Gson gson, ClientManager clientManager) {
        super(socketAddress);
        this.gson = gson;
        this.clientManager = clientManager;
        executorService = Executors.newWorkStealingPool();
    }

    @PostConstruct
    public void init() {
        start();
    }

    @PreDestroy
    public void destroy() {
        executorService.shutdownNow();
        try {
            stop();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onOpen(WebSocket conn, ClientHandshake handshake) {

    }

    @Override
    public void onClose(WebSocket conn, int code, String reason, boolean remote) {
        String clientName = clientManager.remove(conn);
        System.out.println("Disconnected: " + clientName);
    }

    @Override
    public void onMessage(WebSocket conn, String json) {
        Message message = gson.fromJson(json, Message.class);
        if (message.getReceiver() != null) {
            if (clientManager.hasClient(conn)) {
                executorService.submit(() -> clientManager.prepareMessage(message, webSocket -> webSocket.send(json)));
            } else if (message.getReceiver().equals("server") && message.getBody() != null) {
                if (message.getSender() != null && !clientManager.hasClient(message.getSender())) {
                    clientManager.add(conn, message.getSender());
                    System.out.println("Connected: " + message.getSender());
                }
            }
        }
    }

    @Override
    public void onError(WebSocket conn, Exception ex) {

    }
}
