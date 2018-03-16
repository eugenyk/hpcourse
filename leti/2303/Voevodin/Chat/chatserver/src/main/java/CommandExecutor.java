
import chat.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;

import java.util.concurrent.ConcurrentLinkedQueue;//неблокирующая, основана на алгоритме Скотта
public enum CommandExecutor
{
    INSTANCE;
    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private ConcurrentLinkedQueue <Pair> queue;
    private Thread thread;

    CommandExecutor()
    {
        queue = new ConcurrentLinkedQueue <>();
        thread = new Thread(new Task());
    }


    public void addTask(String cmd, Client out)
    {
        Pair pair = new Pair(out, cmd);


            queue.offer(pair);



        if (thread.getState() == Thread.State.NEW)
        {
            thread.start();
        }
    }

    private class Task implements Runnable
    {
        @Override
        public void run()
        {
            while (true)
            {
                 commandHandler(queue.poll());
            }
        }

        private void commandHandler(Pair pair)
        {
            try
            {
                String command = pair.command.substring(3, pair.command.length());
                //logger.info("do Command {}", command);
                Runtime runtime = Runtime.getRuntime();
                Process process = runtime.exec(command);
                BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()));
                String line;
                while ((line = br.readLine()) != null)
                {
                    byte[] callback = Message.Msg.newBuilder()
                            .setSender("Server")
                            .setText(line)
                            .build().toByteArray();

                    ByteBuffer byteBuffer = ByteBuffer.allocate(4 + callback.length);
                    byteBuffer.putInt(callback.length);
                    byteBuffer.put(callback);
                    byteBuffer.rewind();
                    pair.client.write(byteBuffer.array());
                }
                br.close();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    private class Pair
    {
        public Client client;
        public String command;

        public Pair(Client client, String command)
        {
            this.client = client;
            this.command = command;
        }
    }
}
