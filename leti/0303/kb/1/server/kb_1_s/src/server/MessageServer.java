package server;

import server.proto.MessageBody;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.concurrent.*;
public class MessageServer implements CompletionHandler<AsynchronousSocketChannel, Void> {
	public final static int BUFFERSIZE = 8192;

	private int nThreads;
	
	private AsynchronousChannelGroup group;
	private AsynchronousServerSocketChannel server;
	private ConcurrentHashMap<String, AsynchronousSocketChannel> connections;
	private InetSocketAddress address;
	private ConsoleExecutor executor;
	
	public MessageServer(InetSocketAddress address, int numberOfThreads, ConsoleExecutor executor) {
		nThreads = numberOfThreads;
		this.address = address;
		this.executor = executor;

		connections = new ConcurrentHashMap<String, AsynchronousSocketChannel>();
		
		try {
			group = AsynchronousChannelGroup.withFixedThreadPool(nThreads, Executors.defaultThreadFactory());
			server = AsynchronousServerSocketChannel.open(group).bind(address);
			
		}
		catch(Exception e){
			System.out.println("Server making exception. " + e.toString());
		}
	}
	
	public void start() {
		server.accept(null, this);
		System.out.println("Server started.");
	}

	@Override
	public void completed(AsynchronousSocketChannel client, Void arg1) {
		try {
			System.out.println("Connection established. Remote IP:" + client.getRemoteAddress().toString());
			connections.put(client.getRemoteAddress().toString(), client);
		}
		catch(Exception e) {}	
		
		server.accept(null, this);
		
		Reciever reciever = new Reciever(connections, client, executor);
		reciever.start();
		
	}

	@Override
	public void failed(Throwable arg0, Void arg1) {
		// TODO Auto-generated method stub
		
	}

	

}
