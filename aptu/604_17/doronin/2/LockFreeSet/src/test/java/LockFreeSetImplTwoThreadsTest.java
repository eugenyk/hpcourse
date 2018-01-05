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
            IntStream.range(0, 100).forEach(y -> {
                        IntStream.range(0, 100).forEach(x -> set.add(x));
                        Assert.assertFalse(set.isEmpty());
                    }
            );
        });

        Thread deleter = new Thread(() -> {
            IntStream.range(0, 100).forEach(y ->
                    IntStream.range(0, 100).filter(x -> x % 2 == 0).forEach(x -> set.remove(x))
            );
        });

        inserter.start();
        deleter.start();

        inserter.join();
        deleter.join();

        Assert.assertFalse(set.isEmpty());
        for (int i = 0; i < 100; i++) {
            if (i % 2 == 1) {
                Assert.assertTrue(set.contains(i));
            }
        }
    }

    @Test
    public void testMultipleThreadRandom() throws InterruptedException {
        ArrayList<Thread> inserters = new ArrayList<>();

        for (int i = 0; i < Runtime.getRuntime().availableProcessors(); i++) {
            inserters.add(new Thread(() -> {
                while (barrier) ;
                IntStream.range(0, 100).forEach(y ->
                        IntStream.range(0, 100).forEach(x -> {
                            int value = Math.abs(random.nextInt()) % 100;
                            if (set.add(value)) {
                                sum.addAndGet(value);
                            }
                        })
                );
            }));
        }

        ArrayList<Thread> deleters = new ArrayList<>();
        for (int i = 0; i < Runtime.getRuntime().availableProcessors(); i++) {
            deleters.add(new Thread(() -> {
                while (barrier) ;
                IntStream.range(0, 100).forEach(y ->
                        IntStream.range(0, 100).forEach(x -> {
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

        Assert.assertTrue(sum.get() - sub.get() >= 0);
    }
}