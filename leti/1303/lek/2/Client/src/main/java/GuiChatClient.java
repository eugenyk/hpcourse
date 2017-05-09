import protobuf.Message;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * Created by Елена on 07.01.2017.
 */
public class GuiChatClient implements Client {

    // Various GUI components and info
    private JFrame mainFrame = null;
    private JTextArea chatText = null;
    private JTextField chatLine = null;
    private JLabel statusBar = null;
    private JTextField ipField = null;
    private JTextField portField = null;
    private JTextField nameField = null;
    private JButton connectButton = null;
    private JButton disconnectButton = null;

    private ChatClient client = null;

    // Connection info
    public Config defaultConfig = new Config();

    private JPanel initOptionsPane() {
        JPanel pane = null;
        ActionAdapter buttonListener = null;

        // Create an options pane
        JPanel optionsPane = new JPanel(new GridLayout(6, 1));

        // IP address input
        pane = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        pane.add(new JLabel("Host IP:"));
        ipField = new JTextField(20);
        ipField.setText(defaultConfig.getHost());
        ipField.setEditable(true);
        pane.add(ipField);
        optionsPane.add(pane);

        // Port input
        pane = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        pane.add(new JLabel("Port:"));
        portField = new JTextField(20);
        portField.setEditable(true);
        portField.setText((new Integer(defaultConfig.getPort())).toString());
        pane.add(portField);
        optionsPane.add(pane);

        // Port input
        pane = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        pane.add(new JLabel("Name:"));
        nameField = new JTextField(20);
        nameField.setEditable(true);
        nameField.setText("");
        pane.add(nameField);
        optionsPane.add(pane);
        optionsPane.add(Box.createVerticalGlue());
        optionsPane.add(Box.createVerticalGlue());
        optionsPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        // Connect/disconnect buttons
        JPanel buttonPane = new JPanel(new GridLayout(1, 2, 10, 10));

        buttonListener = new ActionAdapter() {
            public void actionPerformed(ActionEvent e) {
                // Request a connection initiation
                if (e.getActionCommand().equals("connect")) {
                    client = new ChatClient(nameField.getText(), GuiChatClient.this);
                    if (client.start()) {
                        connectButton.setEnabled(false);
                        disconnectButton.setEnabled(true);
                        ipField.setEnabled(false);
                        portField.setEnabled(false);
                        nameField.setEnabled(false);
                        chatLine.setEnabled(true);

                        statusBar.setText("Online");
                        mainFrame.repaint();
                    } else {
                        statusBar.setText("It's impossible to set connection!");
                    }
                }
                // Disconnect
                else {
                    disconnect();
                }
            }
        };
        connectButton = new JButton("Connect");
        connectButton.setMnemonic(KeyEvent.VK_C);
        connectButton.setActionCommand("connect");
        connectButton.addActionListener(buttonListener);
        connectButton.setEnabled(true);
        disconnectButton = new JButton("Disconnect");
        disconnectButton.setMnemonic(KeyEvent.VK_D);
        disconnectButton.setActionCommand("disconnect");
        disconnectButton.addActionListener(buttonListener);
        disconnectButton.setEnabled(false);
        buttonPane.add(connectButton);
        buttonPane.add(disconnectButton);
        optionsPane.add(buttonPane);

        return optionsPane;
    }

    private void disconnect() {
        if (client != null) {
            client.disconnect();
        }
        connectButton.setEnabled(true);
        disconnectButton.setEnabled(false);
        ipField.setEnabled(true);
        portField.setEnabled(true);
        nameField.setEnabled(true);
        chatLine.setText("");
        chatLine.setEnabled(false);
        statusBar.setText("Offline");
        mainFrame.repaint();
    }

    private void initGUI() {
        // Set up the status bar
        statusBar = new JLabel();
        statusBar.setText("Offline");

        // Set up the options pane
        JPanel optionsPane = initOptionsPane();

        // Set up the chat pane
        JPanel chatPane = new JPanel(new BorderLayout());
        chatPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        chatText = new JTextArea(10, 20);
        chatText.setLineWrap(true);
        chatText.setEditable(false);
        chatText.setForeground(Color.blue);
        JScrollPane chatTextPane = new JScrollPane(chatText,
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        chatLine = new JTextField();
        chatLine.setEnabled(false);
        Action action = new AbstractAction() {
            public void actionPerformed(ActionEvent e) {
                client.send(chatLine.getText());
                DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm");
                chatText.append(new StringBuffer("You [").append(LocalDateTime.now().format(formatter))
                        .append("] : ").append(chatLine.getText()).append("\n").toString());
                chatLine.setText("");
            }
        };
        chatLine.addActionListener(action);
        chatPane.add(chatLine, BorderLayout.SOUTH);
        chatPane.add(chatTextPane, BorderLayout.CENTER);
        chatPane.setPreferredSize(new Dimension(400, 300));

        // Set up the main pane
        JPanel mainPane = new JPanel(new BorderLayout());
        statusBar.setBorder(BorderFactory.createEmptyBorder(0, 5, 1, 0));
        mainPane.add(statusBar, BorderLayout.SOUTH);
        mainPane.add(optionsPane, BorderLayout.WEST);
        mainPane.add(chatPane, BorderLayout.CENTER);

        // Set up the main frame
        mainFrame = new JFrame("Chat");
        mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        mainFrame.setContentPane(mainPane);
        mainFrame.setSize(mainFrame.getPreferredSize());
        mainFrame.setLocation(200, 200);
        mainFrame.pack();
        mainFrame.setVisible(true);
    }

    public static void main(String args[]) {
        new GuiChatClient().initGUI();
    }

    public void showMessage(Message.Msg message) {
        chatText.append(new StringBuffer(message.getSender()).append(" [").append(message.getDateTime())
                .append("] : ").append(message.getText()).append("\n").toString());
    }

    public void connectionLost() {
        disconnect();
        statusBar.setText("Connection with server was lost!");
    }
}

// Action adapter for easy event-listener coding
class ActionAdapter implements ActionListener {
    public void actionPerformed(ActionEvent e) {
    }
}
