package chat.common;

import java.nio.channels.AsynchronousSocketChannel;
import java.util.Objects;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;

public class ChannelState {

	private AsynchronousSocketChannel channel;

	private final ScheduledExecutorService executor;
	
	private final WriteHandler writeHandler = new WriteHandler(this);		

	private final AtomicBoolean isWait = new AtomicBoolean(false);

	public ChannelState(ScheduledExecutorService executor,
			AsynchronousSocketChannel channel) {
		Objects.nonNull(executor);
		this.executor = executor;
		this.channel = channel;
	}

	public AsynchronousSocketChannel channel() {
		return channel;
	}

	public ChannelState setChannel(AsynchronousSocketChannel channel) {
		this.channel = channel;

		return this;
	}

	public ScheduledExecutorService executor() {
		return executor;
	}

	public AtomicBoolean isWait() {
		return isWait;
	}

	public WriteHandler getWriteHandler() {
		return writeHandler;
	}
}