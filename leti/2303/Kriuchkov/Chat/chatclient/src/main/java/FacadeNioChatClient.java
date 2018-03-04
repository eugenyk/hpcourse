public class FacadeNioChatClient {
    private NotificationsListener notificationsListener;
    private Thread instance;
    private ChatClient chatClient;

    public FacadeNioChatClient(NotificationsListener notificationsListener)
    {
        this.notificationsListener = notificationsListener;
    }

    public void connect(String host, int port, String name)
    {
        chatClient = new ChatClient(host, port, name, notificationsListener);
        instance = new Thread(chatClient);
        instance.start();
        notificationsListener.processConnect();
    }

    public void connect(String name) {
        chatClient = new ChatClient(name, notificationsListener);
        instance = new Thread(chatClient);
        instance.start();
        notificationsListener.processConnect();
    }

    public void disconnect(){
        chatClient.disconnect();
        instance.interrupt();
        while (instance.isInterrupted())
        {
            System.out.println(instance.getState());
        }
        chatClient = null;
    }

    public void send(String msg)
    {
        if (chatClient != null)
        {
            chatClient.send(msg);
        }
    }
}
