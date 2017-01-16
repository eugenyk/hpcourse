package com.github.parfenovvs.websocketchatserver.utils;

import org.java_websocket.WebSocket;

public interface OnPreparedMessage {
    void call(WebSocket webSocket);
}
