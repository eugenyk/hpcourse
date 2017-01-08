import protobuf.Message;

/**
 * Created by Елена on 08.01.2017.
 */
public interface Client {
    void showMessage(Message.Msg message);
    void connectionLost();
}
