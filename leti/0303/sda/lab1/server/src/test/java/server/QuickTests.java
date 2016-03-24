package server;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.function.Consumer;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import chat.client.ChatClient;
import chat.common.Config;
import chat.common.MessageTransfer;
import chat.common.Utils;
import chat.server.ChatServer;

public class QuickTests {

	private static ChatServer chatServer;
	
	private static Config config;

	@BeforeClass
	public static void setUpClass() {
		config = new Config(Utils.getArgsFromSystemProps());
		chatServer = new ChatServer(config);
	}

	@Before
	public void setUp() throws IOException {
		chatServer.start();
	}
	
	@After
	public void tearDown() throws IOException {
		chatServer.stop();
	}
	
	@Test
	public void testSendMessage() throws Exception {
		String mockMessage = "message";
		Consumer<MessageTransfer> consumer =  transfer -> assertEquals(mockMessage, transfer.getText());

		try(ChatClient chatClient= new ChatClient(config, consumer)) {
			chatClient.connect();
			chatClient.sendMessage(mockMessage);
			Thread.sleep(500);			
		}
	}
	

}
