package chat.common;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.CompletionHandler;
import java.nio.channels.InterruptedByTimeoutException;
import java.util.Objects;
import java.util.function.Consumer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ReadHandler implements CompletionHandler<Integer, ChannelState> {

	private static final Logger LOG = LoggerFactory
			.getLogger(ReadHandler.class);

	private final Consumer<byte[]> consumerData;

	private static final Consumer<byte[]> EMPTY_SUPPLIER = raw -> {
	};

	private ByteBuffer buffer = Utils.allocate(4096);

	private byte[] tail = new byte[0];

	private final WaitTask waitTask = new WaitTask();

	private final ChannelState attachment;

	public ReadHandler(ChannelState attachment) {
		this(EMPTY_SUPPLIER, attachment);
	}

	public ReadHandler(Consumer<byte[]> consumerData, ChannelState attachment) {
		Objects.requireNonNull(consumerData);

		this.consumerData = consumerData;
		this.attachment = attachment;
	}

	@Override
	public void completed(Integer result, ChannelState attachment) {
		attachment.isWait().set(false);
		if (result == -1) {
			try {
				attachment.channel().close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return;
		}

		// TODO refactoring
		buffer.flip();
		RAWTokenizer iterator = new RAWTokenizer(Utils.wrap(Utils.concat(tail,
				Utils.getRemaining(buffer))));
		// ByteArraIterable iterator = new RAWTokenizer(buffer()); data < 100
		// 000 Kb
		for (byte[] raw : iterator) {
			consumerData.accept(raw);
		}
		tail = iterator.getTail();

		buffer().clear();
		// buffer().put(tail);

		if (Thread.currentThread().isInterrupted()) {
			return;
		}

		if(attachment.executor().isShutdown()) {
			return;
		}

		attachment.executor().schedule(waitTask, Config.getDelayRead(),
				Config.getTimeUnitDelayRead());
	}

	@Override
	public void failed(Throwable exc, ChannelState attachment) {
		if (exc instanceof InterruptedByTimeoutException) {
			attachment.isWait().set(false);			
			waitTask.run();
		} else {
			LOG.error("Read handler error", exc);
		}
	}

	public ByteBuffer buffer() {
		return buffer;
	}

	private final class WaitTask implements Runnable {

		@Override
		public void run() {
			if(Thread.currentThread().isInterrupted()) {
				return;
			}
			
			if (!Utils.check(attachment)) {
				if (attachment.executor().isShutdown()) {
					return;
				}

				attachment.executor().schedule(waitTask, Config.getDelayRead(),
						Config.getTimeUnitDelayRead());
				return;
			}
			try {
				attachment.channel().read(buffer(), Config.getTimeoutRead(),
						Config.getTimeUnitTimeoutRead(), attachment,
						ReadHandler.this);
			} catch (IllegalStateException e) {
				LOG.error("Illegal state read", e);
				attachment.executor().schedule(waitTask, Config.getDelayRead(),
						Config.getTimeUnitDelayRead());
			}
		}
	}

}
