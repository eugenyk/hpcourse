package chat.common;

import java.io.Serializable;
import java.util.Collection;
import java.util.concurrent.ConcurrentLinkedQueue;

public class User implements Serializable {

	private static final long serialVersionUID = 6691202215273710871L;

	public enum Status {

		CONNECTED,

		DISCONNECTED
	}

	private final String id;

	private Collection<MessageTransfer> messages;

	private Status status = Status.DISCONNECTED;

	private ChannelState channelState;

	public User(String id, ChannelState channelState) {
		super();
		this.id = id;
		this.channelState = channelState;
	}

	public String getId() {
		return id;
	}

	public Collection<MessageTransfer> getMessages() {
		return messages = messages == null ? new ConcurrentLinkedQueue<>()
				: messages;
	}

	public Status getStatus() {
		return status;
	}

	public User setStatus(Status status) {
		this.status = status;

		return this;
	}

	public ChannelState channelState() {
		return channelState;
	}

	public User setChannelState(ChannelState channelState) {
		this.channelState = channelState;

		return this;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((id == null) ? 0 : id.hashCode());
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
		User other = (User) obj;
		if (id == null) {
			if (other.id != null)
				return false;
		} else if (!id.equals(other.id))
			return false;
		return true;
	}

}
