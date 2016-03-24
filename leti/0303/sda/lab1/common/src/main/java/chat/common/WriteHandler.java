package chat.common;

import java.nio.ByteBuffer;
import java.nio.channels.InterruptedByTimeoutException;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class WriteHandler implements CompletationWriteHandler {

	private static final Logger LOG = LoggerFactory
			.getLogger(WriteHandler.class);

	private final Queue<byte[]> queueRAWDataForSend = new ConcurrentLinkedQueue<>();

	private ChannelState attachment;

	private final WaitTask waitTask = new WaitTask();

	private ByteBuffer buffer;

	private AtomicBoolean isSend = new AtomicBoolean(false);

	public WriteHandler(ChannelState attachment) {
		this.attachment = attachment;
	}

	@Override
	public void completed(Integer result, ChannelState attachment) {
		attachment.isWait().set(false);
		buffer = null;

		while (!queueRAWDataForSend.isEmpty()) {
			isSend.set(true);
			buffer = Utils.wrapData(queueRAWDataForSend.poll());
			buffer.flip();

			waitTask.run();
		}

		isSend.set(false);
	}

	@Override
	public void failed(Throwable exc, ChannelState attachment) {
		if (exc instanceof InterruptedByTimeoutException) {
			isSend.set(false);
			attachment.isWait().set(false);
			waitTask.run();
		} else {
			LOG.error("Read handler error", exc);
		}
	}

	@Override
	public void addData(byte[] raw) {
		if (!isSend.get()) {
			buffer = Utils.wrapData(raw);
			buffer.flip();
			waitTask.run();

			return;
		}

		queueRAWDataForSend.add(raw);
	}

	private final class WaitTask implements Runnable {

		@Override
		public void run() {
			if (!Utils.check(attachment)) {
				if (attachment.executor().isShutdown()) {
					return;
				}

				attachment.executor().schedule(this, Config.getDelayWrite(),
						Config.getTimeUnitDelayWrite());
				return;
			}

			try {
				attachment.channel().write(buffer, Config.getTimeoutWrite(),
						Config.getTimeUnitTimeoutWrite(), attachment,
						WriteHandler.this);
			} catch (IllegalStateException e) {
				LOG.error("Error write", e);
				attachment.executor().schedule(this, Config.getDelayWrite(),
						Config.getTimeUnitDelayWrite());
			}
		}
	}

}
