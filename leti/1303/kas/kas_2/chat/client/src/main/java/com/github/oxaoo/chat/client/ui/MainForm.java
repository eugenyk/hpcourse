package com.github.oxaoo.chat.client.ui;

import com.github.oxaoo.chat.client.Client;
import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.swing.*;
import javax.swing.text.BadLocationException;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.ExecutionException;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 15.01.2017
 */
public class MainForm {
    private static final Logger LOG = LoggerFactory.getLogger(MainForm.class);

    private static final int DEFAULT_SERVER_PORT = 5432;
    private static final String DEFAULT_SERVER_HOST = "localhost";
    public static final String DEFAULT_NAME = "anonymous";

    private JPanel mainPanel;
    private JButton sendButton;
    private JTextField messageField;
    private JTextPane chatPane;
    private JTextField loginField;
    private JButton connectButton;
    private JPanel messagePanel;
    private JPanel loginPanel;
    private JTextField hostField;
    private JTextField portField;

    private Client client;
    private String name;

    private boolean isConnect;

    public MainForm() {
        this.isConnect = false;

        connectButton.addActionListener(e -> {

            if (!isConnect) {
                this.isConnect = true;
                this.name = this.getName(this.loginField.getText());
                String host = this.getHost(this.hostField.getText());
                int port = this.getServerPort(this.portField.getText());
                LOG.debug("Name: {}", this.name);

                try {
                    this.client = new Client(host, port, this);
                } catch (IOException ex) {
                    LOG.error("Error while create connection. Cause: {}", ex.toString());
                } catch (InterruptedException | ExecutionException ignored) {
                }

                connectButton.setText("Disconnect");
            } else {
                this.isConnect = false;
                try {
                    this.client.closeConnection();
                } catch (IOException ex) {
                    LOG.error("Error close server connection. Cause: {}", ex.toString());
                }
                connectButton.setText("Connect");
            }
        });

        sendButton.addActionListener(e -> {
            String msg = this.messageField.getText();
            Message.ChatMessage.Builder message = Message.ChatMessage.newBuilder();
            message.setText(msg).setSender(name).setData(this.getCurrentTime());
            try {
                client.sendMessage(message.build());
            } catch (IOException ex) {
                LOG.error("Error while send message to server: {}", ex.toString());
            }
        });

        chatPane.setContentType("text/html");
        chatPane.setEditable(false);
    }

    public void appendMessage(String message) {
        try {
            chatPane.getDocument().insertString(0, message + '\n', null);
        } catch (BadLocationException e) {
            LOG.error("Error during append message. Cause:{}", e.toString());
        }
    }

    private String getCurrentTime() {
        SimpleDateFormat sdfDate = new SimpleDateFormat("HH:mm:ss");
        return sdfDate.format(new Date());
    }

    private int getServerPort(String portArg) {
        int port = DEFAULT_SERVER_PORT;
        if (portArg != null && !portArg.isEmpty()) {
            try {
                port = Integer.parseInt(portArg);
            } catch (NumberFormatException ignore) {}
        }
        if (port < 1024 || port > 65535) port = DEFAULT_SERVER_PORT;
        return port;
    }

    private String getHost(String hostArg) {
        String host = DEFAULT_SERVER_HOST;
        if (hostArg != null && !hostArg.isEmpty()) {
            try {
                host = hostArg;
            } catch (NumberFormatException ignore) {}
        }
        if (host.isEmpty()) host = DEFAULT_SERVER_HOST;
        return host;
    }

    private String getName(String name) {
        if (name == null || name.isEmpty()) return DEFAULT_NAME;
        return name;
    }

    private static MainForm initForm() {
        MainForm mainForm = new MainForm();
        JFrame frame = new JFrame("Chat");
        frame.setContentPane(mainForm.mainPanel);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setResizable(false);
        frame.setSize(700, 400);
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
        return mainForm;
    }

    public static void main(String[] args) {
        MainForm mainForm = initForm();
    }
}
