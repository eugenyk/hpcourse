package ChatServer2;
import java.io.IOException;

public class Main {

    public static void main(String[] args) {
        System.out.println("start server and client");
        try {
            Server server = new Server(new ServerConfig());
       } catch (IOException err) {}



    }
}
