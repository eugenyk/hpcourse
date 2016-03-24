package chat.common;

import java.nio.ByteBuffer;
import java.util.Iterator;

public final class RAWTokenizer implements Iterable<byte[]> {

	private final ByteBuffer data;

	private byte[] tail;

	public RAWTokenizer(ByteBuffer data) {
		data.flip();
		this.data = data;
	}

	@Override
	public Iterator<byte[]> iterator() {
		return new ByteArrayIterator();
	}

	private final class ByteArrayIterator implements Iterator<byte[]> {

		private ByteBuffer buffer;

		public ByteArrayIterator() {
			buffer = data.duplicate();
			buffer.rewind();
		}

		@Override
		public boolean hasNext() {
			if (buffer.remaining() < Config.getSizeMessageBuffer()) {
				setTail(Utils.getRemaining(buffer));
				return false;
			}

			int size = buffer.getInt();
			boolean hasNext = size != 0 && buffer.remaining() >= size;
			buffer.position(buffer.position() - Config.getSizeMessageBuffer());
			if (!hasNext) {
				setTail(Utils.getRemaining(buffer));
			}

			return hasNext;
		}

		@Override
		public byte[] next() {
			byte[] raw = new byte[buffer.getInt()];
			buffer.get(raw);
			return raw;
		}
	}

	/**
	 * Implement if necessary compute tail without getting data
	 * 
	 * @param buffer
	 * @return
	 */
	private static byte[] evalTail(ByteBuffer buffer) {
		throw new UnsupportedOperationException();
	}

	private void setTail(byte[] tail) {
		this.tail = tail;
	}

	public byte[] getTail() {
		return tail = tail == null ? evalTail(data) : tail;
	}
}
