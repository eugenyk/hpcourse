package client;

import client.api.Client;
import client.api.ClientListener;
import client.core.ChatClient;
import client.proto.MessageBody;

import javax.swing.*;
import java.awt.*;
import java.io.IOException;


public class ChatFrame extends JFrame implements ClientListener {
    private Container container;
    private ChatPanel chatPanel;
    private Client client;

    public ChatFrame() throws IOException {
        super("Chat client");
        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        setPreferredSize(new Dimension(300, 500));
        setResizable(false);

        init();
        initUI();

        pack();
        setLocationRelativeTo(null);
        setVisible(true);
    }

    private void init() throws IOException {
        client = new ChatClient();
        client.registerListener(this);
    }

    private void initUI() {
        container = getContentPane();
        container.add(new SettingsPanel((ChatClient) client));
    }

    @Override
    public void newMessage(MessageBody.Message message) {
        if (chatPanel != null) {
            chatPanel.printMessage(message);
        }
    }

    @Override
    public void connectionComplete() {
        container.removeAll();
        chatPanel = new ChatPanel((ChatClient) client);
        container.add(chatPanel);
        validate();
        repaint();
    }

    @Override
    public void disconnectComplete() {
        container.removeAll();
        chatPanel = null;
        container.add(new SettingsPanel((ChatClient) client));
        validate();
        repaint();
    }
}
