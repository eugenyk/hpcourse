package chat.common;

import java.io.Serializable;
import java.util.concurrent.atomic.AtomicReference;

public class MessageTransfer implements Serializable {

	private static final long serialVersionUID = -3625935594276038899L;

	public enum Status {
		SENT,

		WAIT_SEND,

		UNKNOWN
	}

	public enum Type {

		COMMAND,

		MESSAGE
	}

	private final String authorId;

	private final Type type;

	private final long dateTimeStampCreation = System.currentTimeMillis();

	private long dateTimeStampSend;

	private String text;

	private String data;

	private final AtomicReference<Status> status = new AtomicReference<>(
			Status.UNKNOWN);

	public MessageTransfer(String authorId, Type type) {
		this.authorId = authorId;
		this.type = type;
	}

	public long getDateTimeStampSend() {
		return dateTimeStampSend;
	}

	public void setDateTimeStampSend(long dateTimeStampSend) {
		this.dateTimeStampSend = dateTimeStampSend;
	}

	public String getText() {
		return text;
	}

	public void setText(String text) {
		this.text = text;
	}

	public String getData() {
		return data;
	}

	public void setData(String data) {
		this.data = data;
	}

	public Status getStatus() {
		return status.get();
	}

	public void setStatus(Status status) {
		this.status.set(status);
	}

	public String getAuthorId() {
		return authorId;
	}

	public long getDateTimeStampCreation() {
		return dateTimeStampCreation;
	}

	public Type getType() {
		return type;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result
				+ ((authorId == null) ? 0 : authorId.hashCode());
		result = prime * result + ((data == null) ? 0 : data.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		MessageTransfer other = (MessageTransfer) obj;
		if (authorId == null) {
			if (other.authorId != null)
				return false;
		} else if (!authorId.equals(other.authorId))
			return false;
		if (data == null) {
			if (other.data != null)
				return false;
		} else if (!data.equals(other.data))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "MessageTransfer [authorId=" + authorId + ", type=" + type
				+ ", dateTimeStampCreation=" + dateTimeStampCreation
				+ ", dateTimeStampSend=" + dateTimeStampSend + ", text=" + text
				+ ", data=" + data + ", status=" + status + "]";
	}

}
