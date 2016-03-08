package chat.common;

import java.util.Optional;
import java.util.function.Function;

import chat.common.protocol.Message;
import chat.common.protocol.Message.Msg.Builder;

public final class Converters {

	private Converters() {
	}

	public static <Input, T> Function<Optional<Input>, T> newConverter(
			Function<Input, T> converterFunc, T defaultValue) {
		return (Optional<Input> value) -> {
			if (value.isPresent()) {
				try {
					return converterFunc.apply(value.get());

				} catch (Exception ex) {
					return defaultValue;
				}
			} else {
				return defaultValue;
			}
		};
	}

	public static Function<Optional<String>, Integer> newConverter(
			Integer defaultValue) {
		return newConverter(Integer::parseInt, defaultValue);
	}

	public static MessageTransfer toMessageTransfer(Message.Msg message) {
		MessageTransfer messageTransfer = new MessageTransfer(
				message.getSender(),
				message.getText().isEmpty() ? MessageTransfer.Type.COMMAND
						: MessageTransfer.Type.MESSAGE);
		messageTransfer.setText(message.getText());
		messageTransfer.setData(message.getData());

		return messageTransfer;
	}

	public static Message.Msg toMessage(User user, String content,
			MessageTransfer.Type type) {
		Builder messageBldr = Message.Msg.newBuilder().setSender(user.getId());

		switch (type) {
		case COMMAND:
			messageBldr.setText("");
			messageBldr.setData(content);
			break;
		case MESSAGE:
			messageBldr.setText(content);
			break;
		default:
			break;
		}

		return messageBldr.build();
	}
	
	public static String trimArgsCommand(String command) {
		int posPrefix = command.indexOf(Commands.PREFIX);
		if (posPrefix == -1) {
			return "";
		}

		int posArgs = command.indexOf(' ', posPrefix);
		if (posArgs == -1 || posArgs - 1 == command.length()) {
			return "";
		}

		return command.substring(posArgs + 1);
	}
	
	public static String trimCommand(String command) {
		int posPrefix = command.indexOf(Commands.PREFIX);
		if (posPrefix == -1) {
			return "";
		}
		
		int posArgs = command.indexOf(' ', posPrefix);
		if (posArgs == -1 || posArgs - 1 == command.length()) {
			return "";
		}		

		return command.substring(posPrefix, posArgs);
	}	
}
