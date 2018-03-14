public interface NotificationsListener
{
    void processMessage(String msg);
    void processDisconnect();
    void processConnect();
}
