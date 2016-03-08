package chat.common;

import java.util.Optional;

public enum Commands {

	LIST_USERS("users"),

	SHUTDOWN("q"),

	CMD("cmd"),

	UNKNOWN("");

	public static final char PREFIX = '-';

	private static final Optional<Commands> EMPTY = Optional.empty();

	private final String strValue;

	private Commands(String value) {
		this.strValue = PREFIX + value;
	}

	@Override
	public String toString() {
		return strValue;
	}

	public static Optional<Commands> valueOfStr(String value) {
		for (Commands command : Commands.values()) {
			if (command.toString().equals(value)) {
				return Optional.of(command);
			}
		}

		return EMPTY;
	}

}
