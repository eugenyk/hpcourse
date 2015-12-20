package chat.chatclient;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.Border;
import org.apache.log4j.Logger;


public class ChatClientGui {
    
    private final Logger logger = Logger.getLogger(ChatClientGui.class);
    
    private JFrame currFrame;
    private final ChatClient client;
    
    public ChatClientGui() {
        client = new ChatClient(new ChatEventsHandler() {
            
            @Override
            public void onConnect() {
                logger.info("Connected..");
                
                currFrame.setVisible(false);
                currFrame = new ChatFrame();
                currFrame.setVisible(true);
            }

            @Override
            public void onDisconnect() {
                logger.info("Disconnected..");
                
                currFrame.setVisible(false);
                currFrame = new ConnectFrame();
                currFrame.setVisible(true);
            }

            @Override
            public void onMessage(String sender, String message) {
                if (!(currFrame instanceof ChatFrame)) {
                    return;
                }
                
                ChatFrame chatFrame = (ChatFrame) currFrame;
                chatFrame.onMessage(String.format("%s: %s\n", sender, message));
            }
            
        });
        
        currFrame = new ConnectFrame();
        currFrame.setVisible(true);
    }
    
    public static void main(String[] args) throws InterruptedException, IOException {
        new ChatClientGui();
    }
    
    private class ConnectFrame extends JFrame { 
        
        private final JButton connectButton;
        private final JTextField hostField;
        private final JLabel hostLabel;
        private final JPanel jPanel1;
        private final JTextField nameField;
        private final JLabel nameLabel;
        private final JTextField portField;
        private final JLabel portLabel;
        
        public ConnectFrame() {
            jPanel1 = new JPanel();
            hostField = new JTextField();
            hostLabel = new JLabel();
            portLabel = new JLabel();
            portField = new JTextField();
            nameLabel = new JLabel();
            nameField = new JTextField();
            connectButton = new JButton();

            hostLabel.setText("Host");
            hostField.setText(ChatClientGui.this.client.getHost());

            portLabel.setText("Port");
            portField.setText(String.valueOf(ChatClientGui.this.client.getPort()));

            nameLabel.setText("Name");
            nameField.setText(ChatClientGui.this.client.getSenderName());

            GroupLayout jPanel1Layout = new GroupLayout(jPanel1);
            jPanel1.setLayout(jPanel1Layout);
            jPanel1Layout.setHorizontalGroup(
                    jPanel1Layout.createParallelGroup(Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                            .addContainerGap()
                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING)
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING)
                                                    .addComponent(portLabel, GroupLayout.PREFERRED_SIZE, 32, GroupLayout.PREFERRED_SIZE)
                                                    .addComponent(hostLabel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                            .addPreferredGap(ComponentPlacement.RELATED))
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                            .addComponent(nameLabel)
                                            .addGap(29, 29, 29)))
                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING)
                                    .addComponent(hostField, GroupLayout.DEFAULT_SIZE, 117, Short.MAX_VALUE)
                                    .addComponent(portField)
                                    .addComponent(nameField)))
            );
            jPanel1Layout.setVerticalGroup(
                    jPanel1Layout.createParallelGroup(Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING, false)
                                    .addComponent(hostField, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                            .addGap(3, 3, 3)
                                            .addComponent(hostLabel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                            .addPreferredGap(ComponentPlacement.RELATED)
                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING, false)
                                    .addComponent(portField)
                                    .addComponent(portLabel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addPreferredGap(ComponentPlacement.RELATED)
                            .addGroup(jPanel1Layout.createParallelGroup(Alignment.LEADING, false)
                                    .addComponent(nameField)
                                    .addComponent(nameLabel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
            );

            connectButton.setText("Connect");

            GroupLayout layout = new GroupLayout(getContentPane());
            getContentPane().setLayout(layout);
            layout.setHorizontalGroup(
                layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                        .addComponent(jPanel1, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(connectButton, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            layout.setVerticalGroup(
                layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(jPanel1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(ComponentPlacement.UNRELATED)
                    .addComponent(connectButton)
                    .addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            final Border normalBorder = BorderFactory.createLineBorder(Color.GRAY);
            final Border errorBorder = BorderFactory.createLineBorder(Color.RED);
            
            setNormalBorderToFields(normalBorder);
            
            connectButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    setNormalBorderToFields(normalBorder);
                    boolean failed = false;
                    
                    String host = hostField.getText();
                    if (host == null || host.trim().length() == 0) {
                        logger.error("Wrong server host..");
                        hostField.setBorder(errorBorder);
                        failed = true;
                    }
                    
                    String portStr = portField.getText();
                    int port = 0;
                    try {
                        port = Integer.parseInt(portStr);
                    } catch(NumberFormatException ex) {
                        logger.error("Wrong server port..");
                        portField.setBorder(errorBorder);
                        failed = true;
                    }
                    
                    String name = nameField.getText();
                    if (name == null || name.trim().length() == 0) {
                        logger.error("Wrong clent name..");
                        nameField.setBorder(errorBorder);
                        failed = true;
                    }
                    
                    if (failed) {
                        logger.error("Unable to connect!");
                        return;
                    } 
                    
                    ChatClientGui.this.client.connect(host, port, name);
                }
            });
            
            setTitle("Connect to server");
            setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            pack();
            setLocationRelativeTo(null);
            setResizable(false);
        }
        
        private void setNormalBorderToFields(Border border) {
            if (hostField.getBorder() != border) {
                hostField.setBorder(border);
            }
            if (portField.getBorder() != border) {
                portField.setBorder(border);
            }
            if (nameField.getBorder() != border) {
                nameField.setBorder(border);
            }
        }
        
    }
    
    private class ChatFrame extends JFrame { 
        
        private final JButton disconnectButton;
        private final JScrollPane jScrollPane1;
        private final JTextArea msgTextArea;
        private final JTextField msgField;
        private final JLabel nameLabel;
        private final JButton sendButton;
        
        public ChatFrame() {
            jScrollPane1 = new JScrollPane();
            msgTextArea = new JTextArea(20, 5);
            msgField = new JTextField();
            nameLabel = new JLabel();
            sendButton = new JButton();
            disconnectButton = new JButton();

            jScrollPane1.setViewportView(msgTextArea);
            msgTextArea.setEditable(false);

            nameLabel.setText(ChatClientGui.this.client.getSenderName());

            sendButton.setText("Send");

            disconnectButton.setText("Disconnect");

            GroupLayout layout = new GroupLayout(getContentPane());
            this.setLayout(layout);
            layout.setHorizontalGroup(layout.createParallelGroup(Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                            .addContainerGap()
                            .addGroup(layout.createParallelGroup(Alignment.TRAILING, false)
                                    .addComponent(jScrollPane1)
                                    .addGroup(layout.createSequentialGroup()
                                            .addComponent(nameLabel)
                                            .addPreferredGap(ComponentPlacement.UNRELATED)
                                            .addComponent(msgField, GroupLayout.PREFERRED_SIZE, 265, GroupLayout.PREFERRED_SIZE)
                                            .addPreferredGap(ComponentPlacement.RELATED)
                                            .addComponent(sendButton)
                                            .addPreferredGap(ComponentPlacement.RELATED)
                                            .addComponent(disconnectButton)))
                            .addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            layout.setVerticalGroup(layout.createParallelGroup(GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                            .addContainerGap()
                            .addComponent(jScrollPane1, GroupLayout.PREFERRED_SIZE, 317, GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(ComponentPlacement.RELATED)
                            .addGroup(layout.createParallelGroup(Alignment.LEADING, false)
                                    .addGroup(layout.createParallelGroup(Alignment.BASELINE)
                                            .addComponent(msgField)
                                            .addComponent(sendButton)
                                            .addComponent(disconnectButton))
                                    .addComponent(nameLabel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            
            sendButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    String message = msgField.getText();
                    if (message == null || message.trim().length() == 0) {
                        logger.error("Message field is empty..");
                        return;
                    }
                    
                    msgField.setText("");
                    ChatClientGui.this.client.send(message);
                }
            });
            
            disconnectButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    ChatClientGui.this.client.disconnect();
                }
            });
            
            setTitle("Chat window");
            setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            pack();
            setLocationRelativeTo(null);
            setResizable(false);
        }
        
        public void onMessage(String msg) {
            msgTextArea.append(msg);
        }
    }
}
