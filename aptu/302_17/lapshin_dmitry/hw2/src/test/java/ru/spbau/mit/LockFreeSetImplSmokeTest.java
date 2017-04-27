package ru.spbau.mit;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;

@RunWith(Parameterized.class)
public class LockFreeSetImplSmokeTest {
    private final int threads;
    private final double addRate;
    private final double removeRate;
    private final int iterations;
    private final int range;

    public LockFreeSetImplSmokeTest(int threads, double addRate, double removeRate, int iterations, int range) {
        if (addRate + removeRate > 1.0)
            throw new IllegalArgumentException();
        this.threads = threads;
        this.addRate = addRate;
        this.removeRate = removeRate;
        this.iterations = iterations;
        this.range = range;
    }

    @Parameterized.Parameters(name="th-s:{0} {1}/{2} x{3} [0..{4}]")
    public static Collection<Object[]> options() {
        int[] threadCounts = {1, 2, 4, 8};
        double[][] rates = {{0.1, 0.1}, {0.1, 0.8}, {0.8, 0.1}, {0.4, 0.4}};
        int[] iterationCounts = {1000000};
        int[] ranges = {2, 10, 10000};

        List<Object[]> opts = new ArrayList<>();
        for (int threadCount: threadCounts) {
            for (double[] rate : rates) {
                for (int count : iterationCounts) {
                    for (int range : ranges) {
                        Object[] row = {threadCount, rate[0], rate[1], count, range};
                        opts.add(row);
                    }
                }
            }
        }
        return opts;
    }

    @Test
    public void smokeTest() throws InterruptedException {
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();
        ExecutorService service = Executors.newCachedThreadPool();

        for (int i = 0; i != threads; i++) {
            final int id = i;
            service.submit(() -> {
                Set<Integer> mySet = new HashSet<>();
                Random random = new Random();
                for (int j = 0; j != iterations; ++j) {
                    double cmd = random.nextDouble();
                    int arg = random.nextInt(range) * id;
                    if (cmd < addRate) {
                        assertEquals(mySet.add(arg), set.add(arg));
                    } else if (cmd > 1 - removeRate) {
                        assertEquals(mySet.remove(arg), set.remove(arg));
                    } else {
                        assertEquals(mySet.contains(arg), set.contains(arg));
                    }
                }
            });
        }

        service.shutdown();
        service.awaitTermination(10, TimeUnit.MINUTES);
    }

}
