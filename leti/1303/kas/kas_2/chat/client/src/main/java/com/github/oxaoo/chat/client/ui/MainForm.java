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

    private JPanel mainPanel;
    private JButton sendButton;
    private JTextField messageField;
    private JTextPane chatPane;
    private JTextField loginField;
    private JButton connectButton;
    private JPanel messagePanel;
    private JPanel loginPanel;

    private Client client;
    private String name;

    public MainForm() {
        connectButton.addActionListener(e -> {
            this.name = this.loginField.getText();
            LOG.info("Name: {}", this.name);

            try {
                this.client = new Client("localhost", 5432, this);
            } catch (IOException ex) {
                LOG.error("Error while create connection. Cause: {}", ex.toString());
            } catch (InterruptedException | ExecutionException ignored) {
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

    private static MainForm initForm() {
        MainForm mainForm = new MainForm();
        JFrame frame = new JFrame("Chat");
        frame.setContentPane(mainForm.mainPanel);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setResizable(false);
        frame.setSize(600, 400);
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
        return mainForm;
    }

    public static void main(String[] args) {
        MainForm mainForm = initForm();
    }
}
