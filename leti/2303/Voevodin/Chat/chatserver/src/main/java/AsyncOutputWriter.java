import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousByteChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
//import java.util.Queue;

import java.util.concurrent.ConcurrentLinkedQueue;//неблокирующая, осонована на алгоритме Скотта

public class AsyncOutputWriter {
    static CompletionHandler<Integer, Connection> handler =
            new CompletionHandler<Integer, Connection>()
            {
                public void completed(Integer bytesWritten, Connection conn)
                {
                    ConcurrentLinkedQueue<ByteBuffer> queue = conn.queue();
                    ByteBuffer buffer;

                        buffer = queue.peek();
                        assert buffer != null;
                        if (!buffer.hasRemaining())
                        {
                            queue.remove();
                            buffer = queue.peek();
                        }


                    if (buffer != null)
                    {
                        conn.channel().write(buffer, conn, this);
                    }
                }


                public void failed(Throwable exc, Connection conn)
                {
                    try
                    {
                        conn.channel().close();
                    } catch (IOException ex)
                    {
                    } finally
                    {
                        // Remote client closed the connection.
                        finishConnection(conn);
                    }
                }


                private void finishConnection(Connection conn)
                {
                    //Queue<ByteBuffer> queue = conn.queue();

                    ConcurrentLinkedQueue<ByteBuffer> queue=conn.queue();
                    ByteBuffer buffer;

                        buffer = queue.peek();
                        assert buffer != null;
                        if (!buffer.hasRemaining())
                        {
                            queue.remove();
                        }

                }
            };

    public static long flushChannel(Connection conn, ByteBuffer bb) throws IOException
    {
        int nWrite = bb.limit();
        // Write Async and ordered to avoid WritePendingException.
        offer(conn, bb);
        return nWrite;

    }

    static void offer(Connection conn, ByteBuffer buffer)
    {
        //Queue<ByteBuffer> queue = conn.queue();
         ConcurrentLinkedQueue<ByteBuffer> queue=conn.queue();
        boolean needToWrite;

            needToWrite = queue.isEmpty();
            queue.offer(buffer);


        if (needToWrite)
        {
            conn.channel().write(buffer, conn, handler);
        }
    }

    public static class Connection
    {


        private ConcurrentLinkedQueue<ByteBuffer> queue = new ConcurrentLinkedQueue<ByteBuffer>();
        private AsynchronousSocketChannel channel;

        public Connection(AsynchronousSocketChannel channel)
        {
            this.channel = channel;
        }

        AsynchronousByteChannel channel()
        {
            return channel;
        }
        ConcurrentLinkedQueue<ByteBuffer> queue()
        {
            return queue;
        }
    }
}
