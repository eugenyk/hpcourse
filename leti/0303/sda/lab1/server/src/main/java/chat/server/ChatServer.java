package chat.server;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;
import java.util.function.Function;

import org.slf4j.LoggerFactory;

import chat.common.ChannelState;
import chat.common.Commands;
import chat.common.Config;
import chat.common.Converters;
import chat.common.LogLambda;
import chat.common.MessageTransfer;
import chat.common.MessageTransfer.Type;
import chat.common.ReadHandler;
import chat.common.User;
import chat.common.Utils;
import chat.common.protocol.Message;

import com.google.protobuf.InvalidProtocolBufferException;

/**
 * @author Dmitrii
 *
 *         Chat Server. Use non blocking IO "NIO2"
 */
public class ChatServer implements AutoCloseable {

    private static final LogLambda LOG = new LogLambda(LoggerFactory.getLogger(ChatServer.class));

	private AsynchronousChannelGroup channelGroup;

	private Map<String, User> mapUsers;

	private final Map<Commands, Function<String, String>> mapCommandsImmutable;

	private static final Function<String, String> UNKNOWN_COMMAND = arg -> "not supported command";

	private ScheduledExecutorService executorCommands;

	private final Config config;

	private AsynchronousServerSocketChannel serverSocketChannel;
	
	private final AtomicBoolean isRun = new AtomicBoolean(false);

	public ChatServer(Config config) {
		this.config = config;

		mapCommandsImmutable = Collections.unmodifiableMap(initMapCommands());
	}

	private Map<Commands, Function<String, String>> initMapCommands() {

		Map<Commands, Function<String, String>> mapCommands = new HashMap<>(
				Utils.capacity(Commands.values().length));

		mapCommands.put(Commands.LIST_USERS,
				arg -> String.join(",", mapUsers.keySet()));
		mapCommands.put(Commands.CMD, arg -> Utils.executeCommand(arg));
		mapCommands.put(Commands.SHUTDOWN, arg -> {
			try {
				stop();
			} catch (Exception e) {
				LOG.error("Error shutdown server", e);
				return e.getMessage();
			}

			return "stopped server";
		});

		return mapCommands;
	}

	private void executeCommand(User user, MessageTransfer messageTransfer) {
		String data = messageTransfer.getData();
		Optional<Commands> command = Commands.valueOfStr(Converters.trimCommand(data));
		command.orElse(Commands.UNKNOWN);

		command.ifPresent(getCommandConsumer(user, data));
		
	}

	private Consumer<Commands> getCommandConsumer(User user, String data) {
		return (Commands c) -> {
			String result = mapCommandsImmutable.getOrDefault(c,
					UNKNOWN_COMMAND).apply(data);

			user.channelState().getWriteHandler().addData(Converters.toMessage(user, result, Type.COMMAND).toByteArray());
		};
	}

	/**
	 * Start server
	 * @throws IOException 
	 * 
	 *
	 */
	public void start() throws IOException {
		stop();

		isRun.set(true);
		mapUsers = new ConcurrentHashMap<>();		
		executorCommands = Executors.newScheduledThreadPool(1);
		try {
			channelGroup = AsynchronousChannelGroup.withFixedThreadPool(
					config.getCountThreads() - 1,
					Executors.defaultThreadFactory());

			serverSocketChannel = AsynchronousServerSocketChannel
					.open(channelGroup).bind(
							new InetSocketAddress(config.getHost(), config
									.getPort()));

			serverSocketChannel.accept(null,
					new CompletionHandler<AsynchronousSocketChannel, Object>() {

						@Override
						public void completed(
								AsynchronousSocketChannel channel,
								Object attachment) {
							try {
								LOG.debug("Connected: host = {}", channel.getRemoteAddress().toString());
							} catch (IOException e) {
								LOG.error("Error definition remote address", e);
							}
							
							ChannelState channelState = new ChannelState(
									executorCommands, channel);
							ReadHandler readHandler = new ReadHandler(
									getConsumerRAW(channelState), channelState);
							channel.read(readHandler.buffer(), channelState,
									readHandler);

							serverSocketChannel.accept(null, this);
						}

						@Override
						public void failed(Throwable exc, Object attachment) {
							LOG.error("Error accept connection", exc);
						}

					});
		} catch (IOException e) {
			executorCommands.shutdown();			
			LOG.error("error start", e);
			throw e;			
		}

		LOG.info("chat server started: port = {}, ", config.getPort());
	}

	private Consumer<byte[]> getConsumerRAW(ChannelState channelState) {
		return (raw) -> {
			Message.Msg message;
			try {
				message = Message.Msg.parseFrom(raw);
			} catch (InvalidProtocolBufferException e) {
				LOG.error("Error protobuf", e);
				channelState.getWriteHandler().addData(
						Converters.toMessage(new User("-1", null),
								e.getLocalizedMessage(),
								MessageTransfer.Type.MESSAGE).toByteArray());
				return;
			}

			MessageTransfer messageTransfer = Converters
					.toMessageTransfer(message);
			
			LOG.debug("Server: receive message {}", () -> messageTransfer.toString());
			User user = mapUsers.computeIfAbsent(messageTransfer.getAuthorId(),
					(id) -> new User(id, channelState));
			

			switch (messageTransfer.getType()) {
			case COMMAND:
				executeCommand(user, messageTransfer);
				break;
			case MESSAGE:
				broadcast(raw);
				break;
			default:
				break;
			}

			user.getMessages().add(messageTransfer);
		};
	}

	private void broadcast(byte[] raw) {
		for (User user : mapUsers.values()) {
			user.channelState().getWriteHandler().addData(raw);
		}
	}

	/**
	 * Stop server
	 * 
	 * not thread-safe
	 * @throws IOException 
	 */
	public void stop() throws IOException {
		if(!isRun.get()) {
			return;
		}
		
        channelGroup.shutdownNow();		
		serverSocketChannel.close();

		executorCommands.shutdownNow();

		mapUsers.clear();

		channelGroup = null;
		executorCommands = null;
		mapUsers = null;
		
		LOG.info("chat server stopped");		
	}

	public static void main(String[] args) {
		try (ChatServer chatServer = new ChatServer(new Config(args))) {
			try {
				chatServer.start();
			} catch (IOException e1) {
			}
			try {
				System.in.read();
			} catch (IOException e) {
				LOG.error("Error console IO", e);
			}
		}
	}

	@Override
	public void close() {
		try {
			stop();
		} catch (IOException e) {
			LOG.error("error stop", e);
		}
	}
}
