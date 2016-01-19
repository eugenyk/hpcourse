package ru.spb.leti.garadagly.chat.client.core;

public interface NotificationsListener {
    void processMessage(String msg);
    void processDisconnect();
    void processConnect();
}
