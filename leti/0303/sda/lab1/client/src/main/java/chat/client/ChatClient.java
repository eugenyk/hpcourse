package chat.client;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.UUID;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.function.Consumer;

import org.slf4j.LoggerFactory;

import chat.common.ChannelState;
import chat.common.Commands;
import chat.common.Config;
import chat.common.Converters;
import chat.common.LogLambda;
import chat.common.MessageTransfer;
import chat.common.ReadHandler;
import chat.common.User;
import chat.common.protocol.Message;

import com.google.protobuf.InvalidProtocolBufferException;

public class ChatClient implements AutoCloseable {

    private static final LogLambda LOG = new LogLambda(LoggerFactory.getLogger(ChatClient.class));
    
    private final User user;

    private final Config config;

    private final AsynchronousChannelGroup channelGroup;

    private ReadHandler readHandler;
    
    private ScheduledExecutorService executor;
    
    private final Consumer<MessageTransfer> listener;
    
    private static final Consumer<MessageTransfer> EMPTY_LISTENER = d -> {};

    public ChatClient(Config config) throws IOException {
	this(config, EMPTY_LISTENER);
    }
    
    public ChatClient(Config config, Consumer<MessageTransfer> listener) throws IOException {
	this(config, AsynchronousChannelGroup.withFixedThreadPool(1,
		Executors.defaultThreadFactory()), listener);
    }    

    public ChatClient(Config config, AsynchronousChannelGroup group, Consumer<MessageTransfer> listener) {
	this.config = config;
	this.channelGroup = group;
	user = new User(UUID.randomUUID().toString(), null);
	this.listener = listener;
    }

    public void connect() {
	if (user.getStatus().equals(User.Status.CONNECTED)) {
	    disconnect();
	}

	try {
	    AsynchronousSocketChannel channel = AsynchronousSocketChannel.open(channelGroup);
	    executor = Executors.newScheduledThreadPool(1);	    
	    user.setChannelState(new ChannelState(executor, channel));
	    channel.connect(new InetSocketAddress(config.getHost(), config.getPort())).get();
	    user.setStatus(User.Status.CONNECTED);

	    readHandler = new ReadHandler(getConsumerRAW(user, listener), user.channelState());	    
	    user.channelState().channel().read(readHandler.buffer(), Config.getTimeoutRead(), Config.getTimeUnitTimeoutRead(), user.channelState(), readHandler);
	} catch (IOException | InterruptedException | ExecutionException e) {
	    LOG.error("Error connect", e);
	}
    }

    
    private static Consumer<byte[]> getConsumerRAW(User user, Consumer<MessageTransfer> listener) {
	return (raw) -> {
	    Message.Msg message;
	    try {
		message = Message.Msg.parseFrom(raw);
	    } catch (InvalidProtocolBufferException e) {
		e.printStackTrace();
		return;
	    }

	    MessageTransfer messageTransfer = Converters.toMessageTransfer(message);
	    listener.accept(messageTransfer);
	    LOG.debug( "Client receive message from user [{}]: {}", user::getId, messageTransfer::toString);
	    user.getMessages().add(messageTransfer);
	};
    }

    private void sendMessage(Message.Msg msg) {
	if(user.getStatus().equals(User.Status.DISCONNECTED)) {
	    connect();
	}
	
	byte[] raw = msg.toByteArray();
	user.channelState().getWriteHandler().addData(raw);
    }

    public void sendMessage(String message) {
	Message.Msg msg = Message.Msg.newBuilder().setSender(user.getId()).setText(message).build();
	sendMessage(msg);
    }

    public void sendCommand(Commands command, String arg) {
	Message.Msg msg = Message.Msg.newBuilder().setSender(user.getId()).setText("")
		.setData(command.toString() + ' ' + arg).build();
	sendMessage(msg);
    }

    public void disconnect() {
	executor.shutdownNow();
	AsynchronousSocketChannel socketChannel = user.channelState().channel();
	if (socketChannel == null || socketChannel.isOpen()) {
	    return;
	}

	try {
	    socketChannel.close();
	} catch (IOException e) {
	    LOG.error("Error disconnect", e);	    
	}

	user.setChannelState(null);	
	user.setStatus(User.Status.DISCONNECTED);
	readHandler = null;

	LOG.debug("Chat client disconnect");
    }

    @Override
    public void close() throws Exception {
	disconnect();
    }
}
