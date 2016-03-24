package chat.common;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public final class Utils {
	
	private static final Logger LOG = LoggerFactory.getLogger(Utils.class);

	private Utils() {
	}
	
	
	public static String executeCommand(String arg) {
		ProcessBuilder builder = new ProcessBuilder("cmd");
		try {
			Process p = builder.start();
			BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));
			BufferedReader er = new BufferedReader(new InputStreamReader(p.getErrorStream()));
			
			p.getOutputStream().write(Converters.trimArgsCommand(arg + '\n').getBytes(StandardCharsets.UTF_8));
			p.getOutputStream().flush();				
			p.getOutputStream().close();

			if(!p.waitFor(Config.getCommandProcessTimeout(), Config.getTimeUnitCommandProcessTimeOut())) {
				p.destroyForcibly();
				p.waitFor(1, TimeUnit.SECONDS);
			}


			String result = Stream.concat(br.lines(), er.lines().map(line -> "error: " + line)).collect(Collectors.joining("\n"));

			if(!p.isAlive()) {
				result += "\n result code: " + p.exitValue();
			}

			return result;
		} catch (Exception e) {
			LOG.error("Error execute command", e);

			return e.getMessage();
		}		
	}

	public static String[] getArgsFromSystemProps() {
		return System.getProperties().entrySet().stream().
				map(entry -> entry.getKey().toString() +'=' + entry.getValue().toString()).
				collect(Collectors.toSet()).
				toArray(new String[0]);
		
	}

	public static boolean check(ChannelState attachment) {
		boolean resultCompare = false;
		for (int i = 0; !(resultCompare = attachment.isWait().compareAndSet(
				false, true))
				&& i < 5; i++) {
		}

		return resultCompare;
	}

	public static byte[] getRemaining(ByteBuffer buffer) {
		byte[] b = new byte[buffer.remaining()];
		buffer.get(b);

		return b;
	}

	public static ByteBuffer wrapData(byte[] data) {
		return ByteBuffer.allocate(data.length + Config.getSizeMessageBuffer())
				.putInt(data.length).put(data);
	}

	public static ByteBuffer wrap(byte[] raw) {
		return ByteBuffer.allocate(raw.length).put(raw);
	}

	/**
	 * Concat byte array
	 * 
	 * @param arrays
	 * @return concantanation arguments
	 */
	public static byte[] concat(byte[]... arrays) {
		int length = 0;
		for (byte[] array : arrays) {
			length += array.length;
		}
		byte[] result = new byte[length];
		int pos = 0;
		for (byte[] array : arrays) {
			for (byte element : array) {
				result[pos] = element;
				pos++;
			}
		}
		return result;
	}

	public static ByteBuffer allocate(int size) {
		return ByteBuffer.allocate(size).order(Config.getByteOrder());
	}

	public static int getHeaderSizeMessage(ByteBuffer buffer) {
		buffer.flip();
		int size = buffer.getInt();
		return size;
	}

	public static int getSizeMessage(ByteBuffer buffer) {
		return buffer.limit() - Config.getSizeMessageBuffer();
	}

	public static byte[] getRAWMessage(ByteBuffer buffer, int size) {
		byte[] raw = Arrays.copyOfRange(buffer.array(),
				Config.getSizeMessageBuffer(),
				size + Config.getSizeMessageBuffer());
		return raw;
	}

	public static int capacity(int expectedSize) {
		if (expectedSize < 0) {
			throw new IllegalArgumentException("expected size < 0");
		}

		// Avoid the int overflow if expectedSize > (Integer.MAX_VALUE / 2)
		return saturatedCast(Math.max(expectedSize * 2L, 16L));
	}

	private static int saturatedCast(long value) {
		if (value > Integer.MAX_VALUE) {
			return Integer.MAX_VALUE;
		}
		if (value < Integer.MIN_VALUE) {
			return Integer.MIN_VALUE;
		}
		return (int) value;
	}
}
