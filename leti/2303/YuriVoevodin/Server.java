package ChatServer2;
import java.util.logging.Level;
import java.util.logging.Logger;
import ChatServer2.Message;
import ChatServer2.CompletionHandlerInput;
import java.nio.ByteBuffer;

import java.net.InetSocketAddress;
import java.net.StandardSocketOptions;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;

import java.util.concurrent.ConcurrentHashMap;
import java.io.IOException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;

import static ChatServer2.CompletionHandlerInput.submitInputTask;
import static ChatServer2.CompletionHandlerOutput.submitOutputTask;
import static ChatServer2.ConvertMessage.receivingBufferFormMessage;

public class Server implements AutoCloseable  {
    public static void main()  {
        try(Server server = new Server(config)){} catch (IOException err) {}

    }
    private static ServerConfig config = new ServerConfig();
    private  AsynchronousChannelGroup channelGroup;//Группировка асинхронных каналов с целью разделения ресурсов
    //по-умолчанию использует системный пул потоков
    private AsynchronousServerSocketChannel serverSocketChannel;//Асинхронный канал для потоковых сокетов слушания
    private ConcurrentHashMap<AsynchronousSocketChannel, Long> connections = new ConcurrentHashMap<>();//Асинхронный канал для потоковых соединительных сокетов.

    public  Server(ServerConfig config) throws IOException{
        System.out.println("server was started");
        //создали, а точнее переиницировали пул потоков
      channelGroup =  AsynchronousChannelGroup.withFixedThreadPool(config.getThreads(),Executors.defaultThreadFactory());
      //создаем канал через open , и настраиваем на прослушивание соединений
      serverSocketChannel=AsynchronousServerSocketChannel.open(channelGroup).setOption(StandardSocketOptions.SO_REUSEADDR,true).bind(new InetSocketAddress(config.getHost(),config.getPort()));
      //бесконечно как красавчики крутим сервер

        while(true){
            try{
                //инициирую приём подключений
            AsynchronousSocketChannel socketChannel = serverSocketChannel.accept().get();
                connections.put(socketChannel, System.currentTimeMillis());


                submitInputTask(socketChannel,message -> sendAllConnections(message),result -> connections.remove(socketChannel));
            }
            catch (InterruptedException | ExecutionException err){}



        }


    }
    private void sendAllConnections(Message.Msg message) {
        ByteBuffer buffer = receivingBufferFormMessage(message);
        connections.forEachKey(1,socketChannel -> submitOutputTask(socketChannel, buffer, throwable -> System.out.println()));
    }

    public void close() throws IOException {channelGroup.shutdownNow();}

}

