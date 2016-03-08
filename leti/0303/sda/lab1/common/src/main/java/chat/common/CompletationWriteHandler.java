package chat.common;

import java.nio.channels.CompletionHandler;

public interface CompletationWriteHandler extends CompletionHandler<Integer, ChannelState> {

    void addData(byte[] raw);

}