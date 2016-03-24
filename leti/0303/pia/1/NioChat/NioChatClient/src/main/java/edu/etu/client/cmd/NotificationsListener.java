package edu.etu.client.cmd;

public interface NotificationsListener
{
    void processMessage(String msg);
    void processDisconnect();
    void processConnect();
}
