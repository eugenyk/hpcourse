package com.github.oxaoo.chat.server;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 14.01.2017
 */
public class Main {
    private static final int DEFAULT_POOL_SIZE = 10;
    private static final int DEFAULT_PORT = 5432;
    private static final String DEFAULT_HOST = "localhost";

    private static final Logger LOG = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) throws InterruptedException {
        final int poolSize = getPoolSize(args);
        final int port = getPort(args);
        final String host = getHost(args);

        final Server server = new Server(poolSize);

        try {
            server.start(host, port);
        } catch (IOException e) {
            LOG.error("Error while starting server. Cause: {}", e.toString());
        }
    }


    private static int getPoolSize(String[] args) {
        int poolSize = DEFAULT_POOL_SIZE;
        if (args.length > 0) {
            try {
                poolSize = Integer.parseInt(args[0]);
            } catch (NumberFormatException ignore) {}
            if (poolSize < 1)
                poolSize = DEFAULT_POOL_SIZE;
            else if (poolSize > Integer.MAX_VALUE / 2)
                poolSize = Integer.MAX_VALUE / 2;
        }
        return poolSize;
    }

    private static int getPort(String[] args) {
        int port = DEFAULT_PORT;
        if (args.length > 1) {
            try {
                port = Integer.parseInt(args[1]);
            } catch (NumberFormatException ignore) {}
        }
        if (port < 1024 || port > 65535) port = DEFAULT_PORT;
        return port;
    }

    private static String getHost(String[] args) {
        String host = DEFAULT_HOST;
        if (args.length > 2) {
            try {
                host = args[2];
            } catch (NumberFormatException ignore) {}
        }
        if (host.isEmpty()) host = DEFAULT_HOST;
        return host;
    }
}
