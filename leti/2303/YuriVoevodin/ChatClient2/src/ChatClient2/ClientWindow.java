package ChatClient2;

import ChatClient2.Message;
import ChatClient2.Message;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.Closeable;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Random;
import java.util.concurrent.ExecutionException;
import java.util.logging.Level;
import java.util.logging.Logger;

import static ChatClient2.CompletionHandlerInput.submitInputTask;
import static ChatClient2.CompletionHandlerOutput.submitOutputTask;
//import static ru.leti.chat.utils.handlersIO.CompletionHandlerInput.submitInputTask;
//import static ru.leti.chat.utils.handlersIO.CompletionHandlerOutput.submitOutputTask;


public class ClientWindow extends JFrame implements ActionListener, Closeable {
    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Need use - Client <host> <port>");
            return;
        }

        SwingUtilities.invokeLater(new Runnable() {
            private String host;
            private int port;

            private Runnable init(String host, int port) {
                this.host = host;
                this.port = port;
                return this;
            }

            @Override
            public void run() {
                try  {
                    new ClientWindow(host, port);
                } catch (IOException | ExecutionException | InterruptedException e) {
                    log.log(Level.SEVERE, "Client is closed", e);
                }
            }
        }.init(args[0], Integer.parseInt(args[1])));
    }

    private static Logger log = Logger.getLogger(ClientWindow.class.getName());
    private final AsynchronousSocketChannel socketChannel = AsynchronousSocketChannel.open();

    //swing UI
    private static final int WIDTH = 600;
    private static final int HEIGHT = 400;
    private final JTextArea chat = new JTextArea();
    private final JTextField fieldInput = new JTextField();
    private final JTextField fieldName;

    public ClientWindow(String host, int port)
            throws IOException, ExecutionException, InterruptedException {
        //swing ui
        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        setSize(WIDTH, HEIGHT);
        setLocationRelativeTo(null);
        setAlwaysOnTop(true);

        chat.setEditable(false);
        chat.setLineWrap(true);
        add(chat, BorderLayout.CENTER);

        Random rand = new Random();
        fieldName = new JTextField("user#" + rand.nextInt(65000));
        add(fieldName, BorderLayout.NORTH);

        fieldInput.addActionListener(this);
        add(fieldInput, BorderLayout.SOUTH);

        setVisible(true);

        socketChannel.connect(new InetSocketAddress(host, port)).get();

        submitInputTask(socketChannel,
                message -> {
                    SwingUtilities.invokeLater(new Runnable() {

                        public void run() {
                            chat.append(message.getSender() + ": " + message.getText() + "\n");
                        }
                    });
                },
                throwable -> log.log(Level.SEVERE, "Error reading message", throwable)
        );
    }


    public void close() throws IOException {
        socketChannel.close();
    }


    public void writeMessage(String text) {
        try {
            submitOutputTask(socketChannel,
                    buildMessage(text),
                    throwable -> log.log(Level.SEVERE, "Error sending message", throwable)
            );
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private Message.Msg buildMessage(String text) {
        return Message.Msg.newBuilder()
                .setSender(fieldName.getText())
                .setText(text)
                .build();
    }

    public void actionPerformed(ActionEvent e) {
        String msg = fieldInput.getText();
        if (msg.equals("")) {
            return;
        }
        fieldInput.setText(null);
        writeMessage(msg);
    }
}