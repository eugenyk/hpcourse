import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Scanner;

public class Main {

    public static void main(String[] args) throws IOException {
        AsynchronousSocketChannel sock=AsynchronousSocketChannel.open();
        sock.connect(new InetSocketAddress("localhost",10000));
        Handler h=new Handler(sock);
        h.start();
        Scanner in = new Scanner(System.in);
        System.out.format("input name:");
        String name = in.nextLine();
        Message.ClientMessage.Builder b = Message.ClientMessage.newBuilder();
        b.setSender(name);
        b.setText("");
        b.setData("auth");
        Message.ClientMessage msg = b.build();
        int len = msg.toByteArray().length;
        ByteBuffer bb = ByteBuffer.allocate(4 + len);
        bb.putInt(len);
        bb.put(msg.toByteArray());
        bb.position(0);
        sock.write(bb);
        while (true)
        {
            String s=in.nextLine();
            if(s.equals("q()"))break;
            Message.ClientMessage.Builder builder = Message.ClientMessage.newBuilder();
            builder.setSender(name);
            builder.setText(s);
            msg = builder.build();
            len = msg.toByteArray().length;
            bb = ByteBuffer.allocate(4 + len);
            bb.putInt(len);
            bb.put(msg.toByteArray());
            bb.position(0);
            sock.write(bb);
        }
        b = Message.ClientMessage.newBuilder();
        b.setSender(name);
        b.setText("");
        b.setData("q");
        msg = b.build();
        len = msg.toByteArray().length;
        bb = ByteBuffer.allocate(4 + len);
        bb.putInt(len);
        bb.put(msg.toByteArray());
        bb.position(0);
        sock.write(bb);
    }
}
