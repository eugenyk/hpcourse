package com.niochat.client;

import com.niochat.common.CallbackInterface;

import javax.swing.*;
import javax.swing.text.BadLocationException;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

public class ChatClientFrame extends JFrame {
    private ClientMain chatClient;
    private JButton sendButton;
    private JTextField textField1;
    private JPanel mainPanel;
    private JTextPane textPane1;
    private JButton connectButton;
    private JLabel status;
    private JTextField hostTF;
    private JTextField portTF;
    private JTextField nameTF;
    private boolean isConnect = false;

    public ChatClientFrame(String title) throws HeadlessException {
        super(title);
        initialGui();
        this.setContentPane(mainPanel);
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.pack();
        this.setVisible(true);
        chatClient = new ClientMain(new CallbackInterface() {
            public void processMessage(String msg) {
                ChatClientFrame.this.appendToPane(textPane1, msg, Color.BLACK, StyleConstants.ALIGN_LEFT);
            }

            public void processDisconnect() {
                connectButton.setText("Connect");
                status.setText("Disconnect");
                sendButton.setEnabled(false);
                textField1.setEnabled(false);
                hostTF.setEnabled(true);
                portTF.setEnabled(true);
                nameTF.setEnabled(true);
                isConnect = false;
            }

            public void processConnect() {
                connectButton.setText("Disconnect");
                status.setText("Connect as " + nameTF.getText());
                sendButton.setEnabled(true);
                textField1.setEnabled(true);
            }
        });
    }

    public static void main(String[] args) {
        new ChatClientFrame("NioChatClient");
    }

    public void initialGui() {
        connectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (!isConnect) {
                    if (checkFields()) {
                        isConnect = true;
                        chatClient.connect(hostTF.getText(), Integer.parseInt(portTF.getText()), nameTF.getText());
                        hostTF.setEnabled(false);
                        portTF.setEnabled(false);
                        nameTF.setEnabled(false);

                    }
                } else {
                    isConnect = false;
                    chatClient.disconnect();
                    hostTF.setEnabled(true);
                    portTF.setEnabled(true);
                    nameTF.setEnabled(true);
                }
            }
        });

        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (!textField1.getText().equals("")) {
                    String message = nameTF.getText() + ": " + textField1.getText() + "\n";
                    ChatClientFrame.this.appendToPane(textPane1, message, Color.RED, StyleConstants.ALIGN_RIGHT);
                    chatClient.send(textField1.getText());
                    textField1.setText("");
                }
            }
        });

        textField1.addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_ENTER) {
                    if (!textField1.getText().equals("")) {
                        sendButton.doClick();
                    }
                }
            }
        });
    }

    private boolean checkFields() {
        if (hostTF.getText().equals("") || portTF.getText().equals("") || nameTF.getText().equals("")) {
            JOptionPane.showMessageDialog(this, "Not all fields are filled");
            return false;
        }

        try {
            Integer.parseInt(portTF.getText());
        } catch (NumberFormatException e) {
            JOptionPane.showMessageDialog(this, "Wrong format of the port");
            return false;
        }
        return true;
    }

    private void appendToPane(JTextPane tp, String msg, Color textColor, int alignment) {
        StyledDocument doc = tp.getStyledDocument();

        SimpleAttributeSet keyWord = new SimpleAttributeSet();
        StyleConstants.setForeground(keyWord, textColor);
        StyleConstants.setBackground(keyWord, Color.YELLOW);
        StyleConstants.setAlignment(keyWord, alignment);

        StyleConstants.setBold(keyWord, true);

        try {
            int length = doc.getLength();
            doc.insertString(doc.getLength(), msg, keyWord);
            doc.setParagraphAttributes(length + 1, 1, keyWord, false);
        } catch (BadLocationException e) {
            e.printStackTrace();
        }

    }
}
