package chat.chatclient;


public interface ChatEventsHandler {
    
    public void onConnect();
    public void onDisconnect();
    public void onMessage(String sender, String message);
    
}
