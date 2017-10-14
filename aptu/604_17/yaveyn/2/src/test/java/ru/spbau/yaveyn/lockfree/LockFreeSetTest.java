package ru.spbau.yaveyn.lockfree;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.atomic.AtomicInteger;

public class LockFreeSetTest {

    private final static int iterCount = 100;

    private LockFreeSetImpl<Integer> list;
    private ArrayList<Thread> threads;

    private Thread getThread(final CyclicBarrier barrier, final Runnable work) {
        return new Thread(() -> {
            try {
                barrier.await();
            } catch (Throwable t) { }
            work.run();
        });
    }

    @Before
    public void setUp() {
        list = new LockFreeSetImpl<>();
        threads = new ArrayList<>(iterCount);
    }

    private void runThreads() {
        threads.forEach(Thread::start);
        threads.forEach((t) -> { try { t.join(); } catch (InterruptedException e) {} });
    }

    @Test
    public void testOneThread() {
        Assert.assertFalse(list.contains(0));
        Assert.assertTrue(list.isEmpty());
        list.add(0);
        Assert.assertTrue(list.contains(0));
        Assert.assertFalse(list.isEmpty());
        list.remove(0);
        Assert.assertFalse(list.contains(0));
        Assert.assertTrue(list.isEmpty());
    }

    @Test
    public void testAdd() {
        CyclicBarrier barrier = new CyclicBarrier(iterCount);
        for (int i = 0; i < iterCount; ++i) {
            final int k = i;
            threads.add(getThread(barrier, () -> list.add(k)));
        }
        for (int i = 0; i < iterCount; ++i) {
            Assert.assertFalse(list.contains(i));
        }
        runThreads();
        for (int i = 0; i < iterCount; ++i) {
            Assert.assertTrue(list.contains(i));
        }
        Assert.assertFalse(list.isEmpty());
    }

    @Test
    public void testContains() {
        for (int i = 0; i < iterCount; ++i) {
            list.add(i);
        }

        AtomicInteger counterContains = new AtomicInteger(0);
        AtomicInteger counterNotContains = new AtomicInteger(0);
        CyclicBarrier barrier = new CyclicBarrier(2 * iterCount);

        for (int i = 0; i < iterCount; ++i) {
            final int k = i;
            threads.add(getThread(barrier, () -> { if (list.contains(k)) { counterContains.incrementAndGet(); } }));
            threads.add(getThread(barrier, () -> { if (!list.contains(k + iterCount)) { counterNotContains.incrementAndGet(); } }));
        }

        runThreads();

        Assert.assertEquals(iterCount, counterContains.get());
        Assert.assertEquals(iterCount, counterNotContains.get());
    }

    @Test
    public void testRemove() {
        for (int i = 0; i < iterCount; ++i) {
            list.add(i);
        }

        AtomicInteger counterContains = new AtomicInteger(0);
        CyclicBarrier barrier = new CyclicBarrier(iterCount);

        for (int i = 0; i < iterCount; ++i) {
            final int k = i;
            threads.add(getThread(barrier, () -> { if (list.remove(k)) { counterContains.incrementAndGet(); } }));
        }

        runThreads();

        Assert.assertEquals(iterCount, counterContains.get());
        Assert.assertTrue(list.isEmpty());
    }

    @Test
    public void testComplex() {
        AtomicInteger counter = new AtomicInteger(0);
        CyclicBarrier barrier = new CyclicBarrier(2 * iterCount);

        for (int i = 0; i < iterCount; ++i) {
            final int k = i;
            threads.add(getThread(barrier, () -> { if (list.remove(k)) { counter.incrementAndGet(); } }));
            threads.add(getThread(barrier, () -> list.add(k)));
        }

        runThreads();

        for (int i = 0; i < iterCount; ++i) {
            if (list.remove(i)) {
                counter.incrementAndGet();
            }
        }

        Assert.assertEquals(iterCount, counter.get());
        Assert.assertTrue(list.isEmpty());
    }
}
