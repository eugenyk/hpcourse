package Basis;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetSocketAddress;
import java.nio.channels.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;

import server.ConsoleExecutor;
import server.MessageServer;

public class EntryPoint {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		int nThreads;
		String ip,port;
		if(args.length >= 2) {
			nThreads = Integer.parseInt(args[0]);
			String[] inetAddress = args[1].split(":");
			if(inetAddress.length < 2) {
				System.out.println("Wrong address");
				return;
			}
			ip = inetAddress[0];
			port = inetAddress[1];
		} else {
			System.out.println("Not enough arguments");
			return;
		}
		ConsoleExecutor executor = new ConsoleExecutor();
		Thread daemon = new  Thread(executor);
		daemon.setDaemon(true);
		daemon.start();
		InetSocketAddress address = new InetSocketAddress(ip, Integer.parseInt(port));
		MessageServer server = new MessageServer(address, nThreads, executor);
		server.start();
		
	}

}
