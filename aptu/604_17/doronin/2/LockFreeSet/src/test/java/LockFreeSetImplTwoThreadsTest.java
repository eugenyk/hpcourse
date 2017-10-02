import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Random;
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.IntStream;

public class LockFreeSetImplTwoThreadsTest {

    private AtomicLong sum = new AtomicLong();
    private AtomicLong sub = new AtomicLong();
    private long res;
    private LockFreeSet<Integer> set;
    private Random random = new Random();
    private volatile boolean barrier = true;

    @Before
    public void setUp() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void testMultipleThread() throws InterruptedException {
        Thread inserter = new Thread(() -> {
            IntStream.range(0, 10000000).forEach(y ->
                    IntStream.range(0, 1000000).peek(x -> set.add(x))
            );
        });

        Thread deleter = new Thread(() -> {
            IntStream.range(0, 10000000).forEach(y ->
                    IntStream.range(0, 1000000).filter(x -> x % 2 == 0).peek(x -> set.remove(x))
            );
        });

        inserter.start();
        deleter.start();

        inserter.join();
        deleter.join();

        IntStream.range(0, 1000000).filter(x -> x % 2 == 0).peek(x -> Assert.assertTrue(set.contains(x)));
    }

    @Test
    public void testMultipleThreadRandom() throws InterruptedException {
        ArrayList<Thread> inserters = new ArrayList<Thread>();

        for (int i = 0; i < Runtime.getRuntime().availableProcessors(); i++) {
            inserters.add(new Thread(() -> {
                while (barrier) ;
                IntStream.range(0, 10000000).forEach(y ->
                        IntStream.range(0, 1000000).peek(x -> {
                            int value = random.nextInt() % 1_000_000_000;
                            if (set.add(value)) {
                                sum.addAndGet(value);
                            }
                        })
                );
            }));
        }

        ArrayList<Thread> deleters = new ArrayList<Thread>();
        for (int i = 0; i < Runtime.getRuntime().availableProcessors(); i++) {
            deleters.add(new Thread(() -> {
                while (barrier) ;
                IntStream.range(0, 10000000).forEach(y ->
                        IntStream.range(0, 1_000_000_000).filter(x -> x % 2 == 0).peek(x -> {
                            if (set.remove(x)) {
                                sub.addAndGet(x);
                            }
                        })
                );
            }));
        }

        for (Thread thread : inserters) {
            thread.start();
        }

        for (Thread thread : deleters) {
            thread.start();
        }

        barrier = false;

        for (Thread thread : inserters) {
            thread.join();
        }

        for (Thread thread : deleters) {
            thread.join();
        }

        IntStream.range(0, 1_000_000_000).peek(x -> {
            if (set.contains(x)) {
                res += x;
            }
        });

        Assert.assertEquals(res, sum.get() - sub.get());
    }


}