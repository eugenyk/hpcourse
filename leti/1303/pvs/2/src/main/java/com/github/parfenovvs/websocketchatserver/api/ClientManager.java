package com.github.parfenovvs.websocketchatserver.api;

import com.github.parfenovvs.websocketchatserver.entity.Message;
import com.github.parfenovvs.websocketchatserver.utils.OnPreparedMessage;
import org.java_websocket.WebSocket;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;

@Component("clientManager")
public class ClientManager {
    private HashMap<WebSocket, String> clients = new HashMap<>();

    public void add(WebSocket webSocket, String clientName) {
        clients.put(webSocket, clientName);
    }

    public boolean hasClient(String clientName) {
        for (Map.Entry<WebSocket, String> client : clients.entrySet()) {
            if (client.getValue().equals(clientName)) {
                return true;
            }
        }
        return false;
    }

    public boolean hasClient(WebSocket webSocket) {
        for (Map.Entry<WebSocket, String> client : clients.entrySet()) {
            if (client.getKey() == webSocket) {
                return true;
            }
        }
        return false;
    }

    public String remove(WebSocket webSocket) {
        return clients.remove(webSocket);
    }

    public void prepareMessage(Message message, OnPreparedMessage sendFunc) {
        for (Map.Entry<WebSocket, String> client : clients.entrySet()) {
            if (message.getReceiver().equals("all")) {
                sendFunc.call(client.getKey());
            } else if (client.getValue().equals(message.getReceiver())) {
                sendFunc.call(client.getKey());
                return;
            }
        }
    }
}
