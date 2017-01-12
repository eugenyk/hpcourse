package com.github.oxaoo.chat.server;

import java.util.concurrent.*;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 12.01.2017
 */
public class Server {
    private static int DEFAULT_POOL_SIZE = 10;

    private ThreadPoolExecutor poolExecutor;

    public static void main(String[] args) {
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

        final Server server = new Server();
        server.start(poolSize);
    }

    void start(int poolSize) {
        int maxPullSize = poolSize * 2;
        this.poolExecutor = new ThreadPoolExecutor(poolSize, maxPullSize, 0, TimeUnit.SECONDS,
                new LinkedBlockingQueue<Runnable>());
    }
}
