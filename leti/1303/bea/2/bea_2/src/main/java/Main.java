import server.Server;

public class Main {
    public static void main(String[] args) throws InterruptedException {
        new Server(8282).start();
    }
}
