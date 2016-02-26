package client;

import client.core.ChatClient;

import javax.swing.*;
import java.awt.*;
import java.io.IOException;


public class SettingsPanel extends JPanel {
    private ChatClient client;
    private JTextField hostField;
    private JTextField portField;
    private JTextField nameField;
    private JButton connectButton;
    private JLabel infoLabel;

    public SettingsPanel(ChatClient client) {
        super();
        this.client = client;

        initUI();
    }

    private void initUI() {
        final JLabel hostLabel = new JLabel("Host");
        final JLabel portLabel = new JLabel("Port");
        final JLabel userLabel = new JLabel("Username");
        hostField = new JTextField("127.0.0.1");
        portField = new JTextField("10000");
        nameField = new JTextField("unknown");
        infoLabel = new JLabel();

        hostField.setPreferredSize(new Dimension(200, 20));
        portField.setPreferredSize(new Dimension(200, 20));
        nameField.setPreferredSize(new Dimension(200, 20));
        hostLabel.setPreferredSize(new Dimension(60, 20));
        portLabel.setPreferredSize(new Dimension(60, 20));
        userLabel.setPreferredSize(new Dimension(60, 20));

        connectButton = new JButton("Connect");
        connectButton.addActionListener(event -> connectHandler());

        add(hostLabel);
        add(hostField);
        add(portLabel);
        add(portField);
        add(userLabel);
        add(nameField);
        add(infoLabel);
        add(connectButton);
    }

    private void connectHandler() {
        String host = hostField.getText();
        String port = portField.getText();
        String name = nameField.getText();

        if (!host.isEmpty()) {
            client.setHost(host);
        }
        if (!port.isEmpty()) {
            try {
                int p = Integer.parseInt(port);
                client.setPort(p);
            } catch (NumberFormatException e) {
                infoLabel.setText("Incorrect port number");
                repaint();
                return;
            }
        }
        if (!name.isEmpty()) {
            client.setClientName(name);
        }

        connecting();
        try {
            client.connect();
        } catch (IOException e) {
            disconnected("Failed connection!");
        }
    }

    private void connecting() {
        SwingUtilities.invokeLater(() -> {
            infoLabel.setText("Connecting...");
            hostField.setEditable(false);
            portField.setEditable(false);
            nameField.setEditable(false);
            connectButton.setVisible(false);
            repaint();
        });
    }

    public void disconnected(String info) {
        infoLabel.setText(info);
        hostField.setEditable(true);
        portField.setEditable(true);
        nameField.setEditable(true);
        connectButton.setVisible(true);
        repaint();
    }
}
