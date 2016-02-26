import java.net.InetSocketAddress;

public class ServerRunner {

    private static final int DEFAULT_NUMBER_THREADS = 1337;

    private static final String DEFAULT_SERVER_HOSTNAME = "localhost";
    private static final int DEFAULT_SERVER_PORT = 10000;

    public static void main(String[] args) {

        InetSocketAddress serverAddress = getServerAddressFromArgs(args);

        int numberOfThreads = getNumberOfThreadsFromArgs(args);

        Server hpserver = new Server(serverAddress, numberOfThreads);

        hpserver.start();
    }

    private static InetSocketAddress getServerAddressFromArgs(String[] args) {
        String serverHostname = getServerHostnameFromArgs(args);
        int serverPort = getServerPortFromArgs(args);

        return new InetSocketAddress(serverHostname, serverPort);
    }

    private static String getServerHostnameFromArgs(String[] args) {
        if (args.length > 0) {
            return args[0];
        } else {
            return DEFAULT_SERVER_HOSTNAME;
        }
    }

    private static int getServerPortFromArgs(String[] args) {
        if (args.length > 1) {
            return Integer.parseInt(args[1]);
        } else {
            return DEFAULT_SERVER_PORT;
        }
    }

    private static int getNumberOfThreadsFromArgs(String[] args) {
        if (args.length > 2) {
            return Integer.parseInt(args[2]);
        } else {
            return DEFAULT_NUMBER_THREADS;
        }
    }


}
