package ChatClient2;


import java.io.IOException;
import java.util.concurrent.ExecutionException;

public class Main {

    public static void main(String[] args) {
	// write your code here
        System.out.println("start server and client");
        try {
            ClientWindow client = new ClientWindow("127.0.0.1", 8888);
       } catch (IOException | ExecutionException | InterruptedException e ) {}



    }
}
