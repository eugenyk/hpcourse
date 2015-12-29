package com.niochat.server;

import com.google.protobuf.InvalidProtocolBufferException;
import com.niochat.common.CallbackInterface;
import com.niochat.common.Client;
import com.niochat.common.Constants;
import com.niochat.common.Processor;
import com.niochat.message.ProtoMessage;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.Properties;
import java.util.concurrent.*;
import java.util.logging.Logger;

public class ServerMain implements Runnable{
    private final static Logger             log = Logger.getLogger(ServerMain.class.getName());
    private String                          host = Constants.host;
    private int                             port = Constants.port;
    private int                             threads = Constants.threads;
    public  final static int                BUFFER_SIZE_IN_BYTES = Constants.BUFFER_SIZE_IN_BYTES;
    private CopyOnWriteArraySet<Client>     clientSet;
    private TaskExecutor                    tEx;

    static ServerMain instance = new ServerMain();

    public static ServerMain getInstance() {
        return instance;
    }

    private ServerMain() {
        new Thread(new TaskExecutor()).start();
    }

    void configure(String configfile) {
        Properties prop = System.getProperties();

        try (InputStream input = new FileInputStream(configfile)) {
            prop.load(input);
        } catch (IOException | NullPointerException e) {
            log.info("Property file was not provided.");
        }

        String host = prop.getProperty("host");
        if (host != null)
            this.host = host;

        try {
            port = Integer.parseInt(prop.getProperty("port"));
        } catch (NumberFormatException e) {
        }
        try {
            threads = Integer.parseInt(prop.getProperty("threads"));
        } catch (NumberFormatException e) {
        }
    }

    @Override
    public void run() {
        try {
            AsynchronousServerSocketChannel sSocketChannel =
                    AsynchronousServerSocketChannel.open(
                            AsynchronousChannelGroup.withFixedThreadPool(threads, Executors.defaultThreadFactory()));
            log.info("Starting with:");
            log.info("Host=" + this.host);
            log.info("Port=" + this.port);
            log.info("Number of threads=" + this.threads);

            clientSet = new CopyOnWriteArraySet<Client>();

            sSocketChannel.bind(new InetSocketAddress(host, port));

            sSocketChannel.accept(null, new CompletionHandler<AsynchronousSocketChannel, Void>() {
                @Override
                public void completed(AsynchronousSocketChannel result, Void attachment) {
                    log.info("Accept client");
                    Client cur = new Client(result, new ServerProc(), new CallbackInterface() {
                        @Override
                        public void processMessage(String msg) {}

                        @Override
                        public void processDisconnect() {}

                        @Override
                        public void processConnect() {}
                    });
                    clientSet.add(cur);
                    sSocketChannel.accept(null, this);
                }

                @Override
                public void failed(Throwable exc, Void attachment) {
                    exc.printStackTrace();
                    try {
                        sSocketChannel.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    log.info("Failed to accept: " + exc.getMessage());
                }
            });
        } catch(IOException e) {
            System.out.println("IOException, server of port " +this.port+ " terminating. Stack trace:");
            e.printStackTrace();
        }
    }

    class ServerProc implements Processor {
        @Override
        public void process(ByteBuffer bf, Client client) {
            try {
                bf.flip();
                int limit = bf.limit();
                byte[] received = new byte[limit];
                bf.get(received);
                System.out.println(new String(received));
                ProtoMessage.Message msg = ProtoMessage.Message.parseFrom(received);

                log.info(msg.getSender() + ": " + msg.getText());
                if (!isCommand(msg.getText())) {
                    broadcast(client, msg.toByteArray());
                } else {
                    tEx.executeTask(client, msg.getText().substring("/command ".length()));
                }
            } catch (InvalidProtocolBufferException e) {
                e.printStackTrace();
            }
        }
    }

    private boolean isCommand(String text) {
        if (text.startsWith("/command "))
            return true;
        return false;
    }

    private void broadcast(Client sender, byte[] msg) {
        Client []notWorking = new Client[clientSet.size()];
        int i = 0;
        for (Client client : clientSet) {
            if (!sender.equals(client)) {
                if (client.isActive())
                    client.write(msg);
                else {
                    notWorking[i++] = client;
                }
            }
        }
        for (int j = 0; i < i; j++) {
            clientSet.remove(notWorking[j]);
        }
    }

    class Task {
        Client client;
        String command;
    }

    class TaskExecutor implements Runnable {
        private BlockingQueue<Task> queue;

        public void executeTask(Client client, String cmd) {
            Task task = new Task();
            task.client = client;
            task.command = cmd;
            try {
                queue.put(task);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void run() {
            while (true) {
                try {
                    commandHandler(queue.take());
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        private void commandHandler(Task task) {
            if (task == null)
                return;
            Process process;
            try {
                process = Runtime.getRuntime().exec(task.command);
            } catch (IOException ex) {
                ex.printStackTrace();
                return;
            }
            try (
                BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                StringBuilder sb = null;
                while ((line = br.readLine()) != null) {
                    sb = new StringBuilder(512);
                    sb.append(line);
                }
                byte[] responce = ProtoMessage.Message.newBuilder()
                        .setSender("Server")
                        .setText(sb.toString())
                        .build().toByteArray();
                task.client.write(responce);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String[] args) throws InterruptedException, IOException {
        ServerMain.getInstance().configure(System.getProperty("configfile"));
        new Thread(ServerMain.getInstance()).start();
        Thread.currentThread().join();
    }
}
