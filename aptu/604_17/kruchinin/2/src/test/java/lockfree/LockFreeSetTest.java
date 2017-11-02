package lockfree;

import org.junit.Before;
import org.junit.Test;

import java.util.Random;
import java.util.concurrent.*;
import java.util.concurrent.locks.ReentrantLock;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetTest {
    private LockFreeSet<Integer> set;

    @Before
    public void setUp() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void testSimple() {
        Integer value = 0;

        assertTrue(set.isEmpty());
        assertFalse(set.contains(value));

        set.add(value);

        assertTrue(set.contains(value));
        assertFalse(set.isEmpty());

        assertTrue(set.remove(value));
        assertFalse(set.remove(value));

        assertFalse(set.contains(value));
        assertTrue(set.isEmpty());
    }

    @Test
    public void testInsertDeleteOneThread() {
        final int numInsertions = 1000;

        assertTrue(set.isEmpty());

        for (int i = 0; i < numInsertions; i++) {
            set.add(i);
        }

        assertFalse(set.isEmpty());

        for (int i = 0; i < numInsertions; i++) {
            assertTrue(set.contains(i));
        }

        assertFalse(set.isEmpty());

        for (int i = 0; i < numInsertions; i++) {
            assertTrue(set.remove(i));
        }

        assertTrue(set.isEmpty());
    }

    @Test
    public void testInsertDeleteMultiThread() throws InterruptedException {
        final int numThreads = 8;
        final int numInsertions = 1000;

        final Thread[] threads = new Thread[numThreads];
        final CyclicBarrier barrierStart = new CyclicBarrier(numThreads);
        final CyclicBarrier barrierAppend = new CyclicBarrier(numThreads);
        final CyclicBarrier barrierContains = new CyclicBarrier(numThreads);
        final CyclicBarrier barrierRemove = new CyclicBarrier(numThreads);

        for (int i = 0; i < threads.length; i++) {
            final int finalI = i;
            threads[i] = new Thread(() -> {
                try {
                    barrierStart.await();

                    for (int j = 0; j < numInsertions; j++) {
                        set.add(finalI * numInsertions + j);
                    }

                    barrierAppend.await();

                    for (int j = 0; j < numThreads * numInsertions; j++) {
                        assertTrue(set.contains(j));
                    }

                    barrierContains.await();

                    for (int j = 0; j < numInsertions; j++) {
                        assertTrue(set.remove(finalI * numInsertions + j));
                    }

                    barrierRemove.await();

                    for (int j = 0; j < numThreads * numInsertions; j++) {
                        assertFalse(set.remove(j));
                    }
                } catch (InterruptedException | BrokenBarrierException ignored) { }
            });
        }

        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }

        assertTrue(set.isEmpty());
    }

    @Test
    public void testConcurrentInsertAndDelete() throws InterruptedException {
        ConcurrentSkipListSet<Integer> concurrentSet = new ConcurrentSkipListSet<>();

        final int numThreads = 8;
        final int numOperations = 1_000_000;
        final Random rng = new Random();
        final int randomBound = 100;

        final Thread[] threads = new Thread[numThreads];
        final ReentrantLock[] locks = new ReentrantLock[randomBound];

        for (int i = 0; i < randomBound; i++) {
            locks[i] = new ReentrantLock();
        }

        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(() -> {
                for (int j = 0; j < numOperations; j++) {
                    boolean toInsert = rng.nextBoolean();
                    final Integer key = rng.nextInt(randomBound);

                    try {
                        locks[key].lock();

                        if (toInsert) {
                            assertEquals(set.add(key), concurrentSet.add(key));
                        } else {
                            assertEquals(set.remove(key), concurrentSet.remove(key));
                        }
                    } finally {
                        locks[key].unlock();
                    }
                }
            });
        }

        for (Thread thread: threads) {
            thread.start();
        }

        for (Thread thread: threads) {
            thread.join();
        }
    }
}
