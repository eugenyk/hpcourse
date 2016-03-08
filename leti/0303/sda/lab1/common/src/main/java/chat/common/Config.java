package chat.common;

import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Optional;
import java.util.Properties;
import java.util.concurrent.TimeUnit;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collectors;

public class Config {

	// variable params
	private static final String PROP_HOST = "h";
	private static final String PROP_HOST_DEFAULT_VALUE = "localhost";

	private static final String PROP_PORT = "p";
	private static final Integer PROP_PORT_DEFAULT_VALUE = 8000;

	private static final String PROP_COUNT_THREADS = "t";
	private static final int COUNT_THREADS_DEFAULT = 2;

	private static final ByteOrder BYTE_ORDER = ByteOrder.BIG_ENDIAN;

	private static final int SIZE_HEADER = 4;

	private static final int TIMEOUT_COMMAND_PROCESS = 5;

	private static final TimeUnit TIMUNIT_TIMEOUT_COMMAND_PROCESS = TimeUnit.SECONDS;

	private static final int TIMEOUT_READ = 200;

	private static final TimeUnit TIMEUNIT_TIMEOUT_READ = TimeUnit.MILLISECONDS;

	private static final int DELAY_READ = 200;

	private static final TimeUnit TIMEUNIT_DELAY_READ = TimeUnit.MILLISECONDS;

	private static final int TIMEOUT_WRITE = 200;

	private static final TimeUnit TIMEUNIT_TIMEOUT_WRITE = TimeUnit.MILLISECONDS;

	private static final int DELAY_WRITE = 200;

	private static final TimeUnit TIMEUNIT_DELAY_WRITE = TimeUnit.MILLISECONDS;

	private final Properties props;

	private final Supplier<String> propHost;

	private final Supplier<Integer> propPort;

	private final Supplier<Integer> propCountThreads;

	public Config(String[] args) {
		props = Arrays
				.stream(args)
				.filter((String arg) -> arg.startsWith("-")
						&& arg.contains("="))
				.collect(
						Collectors.toMap((String arg) -> arg.substring(1,
								arg.indexOf('=')), (String arg) -> arg
								.substring(arg.indexOf('=') + 1, arg.length()),
								(oldV, newV) -> newV, Properties::new));

		propHost = newProp(PROP_HOST, PROP_HOST_DEFAULT_VALUE);

		propPort = newProp(

				PROP_PORT,

				Converters.newConverter(PROP_PORT_DEFAULT_VALUE).andThen(
						(Integer source) -> {
							if (source < 0)
								return PROP_PORT_DEFAULT_VALUE;

							else
								return source;
						}));

		propCountThreads = newProp(

				PROP_COUNT_THREADS,

				Converters.newConverter(COUNT_THREADS_DEFAULT).andThen(
						(Integer source) -> {
							if (source < 0)
								return COUNT_THREADS_DEFAULT;

							else
								return source;
						}));
	}

	public String getHost() {
		return propHost.get();
	}

	public int getPort() {
		return propPort.get();
	}

	public int getCountThreads() {
		return propCountThreads.get();
	}

	public Optional<String> get(String key) {
		return Optional.ofNullable(props.getProperty(key));
	}

	public String get(String key, String defaultValue) {
		return props.getProperty(key, defaultValue);
	}

	public final Supplier<String> newProp(String key, String defaultValue) {
		return () -> get(key, defaultValue);
	}

	public final <T> Supplier<T> newProp(String key,
			Function<Optional<String>, T> converter) {
		return () -> converter.apply(get(key));
	}

	public static final ByteOrder getByteOrder() {
		return BYTE_ORDER;
	}

	public static final int getSizeMessageBuffer() {
		return SIZE_HEADER;
	}

	public static final int getCommandProcessTimeout() {
		return TIMEOUT_COMMAND_PROCESS;
	}

	public static final TimeUnit getTimeUnitCommandProcessTimeOut() {
		return TIMUNIT_TIMEOUT_COMMAND_PROCESS;
	}

	public static final int getTimeoutRead() {
		return TIMEOUT_READ;
	}

	public static final TimeUnit getTimeUnitTimeoutRead() {
		return TIMEUNIT_TIMEOUT_READ;
	}

	public static int getDelayRead() {
		return DELAY_READ;
	}

	public static TimeUnit getTimeUnitDelayRead() {
		return TIMEUNIT_DELAY_READ;
	}

	public static int getDelayWrite() {
		return DELAY_WRITE;
	}

	public static TimeUnit getTimeUnitDelayWrite() {
		return TIMEUNIT_DELAY_WRITE;
	}

	public static int getTimeoutWrite() {
		return TIMEOUT_WRITE;
	}

	public static TimeUnit getTimeUnitTimeoutWrite() {
		return TIMEUNIT_TIMEOUT_WRITE;
	}

}
