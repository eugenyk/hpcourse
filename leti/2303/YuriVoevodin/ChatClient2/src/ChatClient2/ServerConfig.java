package ChatClient2;

public class ServerConfig {
    private String host = "127.0.0.1";
    private int port = 8888;
    private int threads = Runtime.getRuntime().availableProcessors();

    public ServerConfig(){}

    public      final String getHost(){return host;}
    public final int getPort(){return port;}
    public final int getThreads(){return threads;}

}
