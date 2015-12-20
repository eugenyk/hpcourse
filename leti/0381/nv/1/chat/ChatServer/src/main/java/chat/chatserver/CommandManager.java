package chat.chatserver;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import org.apache.log4j.Logger;



public final class CommandManager {
    
    private final Logger logger = Logger.getLogger(CommandManager.class);
    private final BlockingQueue<CommandInfo> commandsQueue;
    private final Thread currentThread;
    
    private static class CommandManagerHolder {
        public static CommandManager instance = new CommandManager();
    }
    
    public static CommandManager getInstance() {
        return CommandManagerHolder.instance;
    }
    
    private CommandManager() {
        commandsQueue = new LinkedBlockingQueue<>(2048);
        currentThread = new Thread(new ProcessingHandler());
        currentThread.start();
    }
    
    public void processCommand(Client target, String command) {
        try {
            commandsQueue.put(new CommandInfo(target, command));
        } catch (InterruptedException ex) {
            logger.error(ex);
        }
    }
    
    private static class CommandInfo {
        public Client target;
        public String command;
        
        public CommandInfo(Client target, String command) {
            this.target = target;
            this.command = command;
        }
    }
    
    private class ProcessingHandler implements Runnable {

        @Override
        public void run() {
            while (true) {
                try {
                    CommandInfo info = commandsQueue.take();
                    handle(info);
                } catch (InterruptedException ex) {
                    logger.error(ex);
                }
            }
        }
        
        private void handle(CommandInfo commandInfo) {
            String command = commandInfo.command.substring(3, commandInfo.command.length());
            
            try {
                Process process = Runtime.getRuntime().exec(command);
                try (BufferedReader bufReader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                    String line;
                    while ((line = bufReader.readLine()) != null) {
                        byte[] answer = Message.Msg.newBuilder()
                                .setSender("SERVER")
                                .setText(line)
                                .build().toByteArray();
                        
                        ByteBuffer byteBuffer = ByteBuffer.allocate(4 + answer.length);
                        byteBuffer.putInt(answer.length);
                        byteBuffer.put(answer);
                        byteBuffer.rewind();
                        
                        commandInfo.target.sendMessage(byteBuffer);
                    }
                }
            } catch (IOException ex) {
                logger.error(String.format("Error while handling command '%s'!", command), ex);
            }
        }
        
    }
}
