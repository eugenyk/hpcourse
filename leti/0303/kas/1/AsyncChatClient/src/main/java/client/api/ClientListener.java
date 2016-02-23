package client.api;

import client.proto.MessageBody;


public interface ClientListener {
    void newMessage(MessageBody.Message message);
    void connectionComplete();
    void disconnectComplete();
}
