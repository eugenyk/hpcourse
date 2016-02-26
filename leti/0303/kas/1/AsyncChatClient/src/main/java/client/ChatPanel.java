package client;

import client.core.ChatClient;
import client.proto.MessageBody;

import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;


public class ChatPanel extends JPanel {
    private final DateTimeFormatter dtf = DateTimeFormatter.ofPattern("dd.MM.yyyy - HH:mm");
    private ChatClient client;
    private JTextArea chatArea;
    private JTextArea messageArea;

    public ChatPanel(ChatClient client) {
        super();
        this.client = client;

        initUI();
    }

    private void initUI() {
        final JLabel userLabel = new JLabel(client.getClientName() + ", you connected.");
        final JButton disconnectButton = new JButton("Exit");
        disconnectButton.addActionListener(event -> disconnectHandler());

        chatArea = new JTextArea(0, 2);
        chatArea.setEditable(false);
        messageArea = new JTextArea();

        final JScrollPane chatPane = new JScrollPane(chatArea);
        final JScrollPane messagePane = new JScrollPane(messageArea);
        final JButton sendButton = new JButton("Send");
        sendButton.addActionListener(event -> sendMessage(messageArea.getText()));

        userLabel.setPreferredSize(new Dimension(200, 20));
        disconnectButton.setPreferredSize(new Dimension(80, 25));
        chatPane.setPreferredSize(new Dimension(280, 280));
        messagePane.setPreferredSize(new Dimension(280, 100));

        add(userLabel);
        add(disconnectButton);
        add(chatPane);
        add(messagePane);
        add(sendButton);
    }

    private void disconnectHandler() {
        try {
            client.disconnect();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void sendMessage(String text) {
        MessageBody.Message msg = MessageBody.Message.newBuilder()
                .setSender(client.getClientName())
                .setText(text)
                .build();

        client.write(msg.toByteArray());
        messageArea.setText("");
        printMessage(msg);
    }

    public void printMessage(MessageBody.Message msg) {
        chatArea.append(msg.getSender() + " (" + LocalDateTime.now().format(dtf) + ") :\n");
        chatArea.append(msg.getText() + "\n");
    }
}
