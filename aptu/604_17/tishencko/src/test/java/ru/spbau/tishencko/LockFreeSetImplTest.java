package ru.spbau.tishencko;

import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.Assert.*;

public class LockFreeSetImplTest {
    private LockFreeSet<Integer> list;
    private ArrayList<Thread> threads;
    private final int threadCount = 1024;

    @Before
    public void initialize() {
        list = new LockFreeSetImpl<Integer>();
        threads = new ArrayList<>();
    }

    @Test
    public void isEmpty() throws Exception {
        assertTrue(list.isEmpty());
        assertTrue(list.add(100));
        assertFalse(list.add(100));
        assertTrue(list.add(20));
        assertFalse(list.isEmpty());
        assertTrue(list.remove(100));
        assertFalse(list.isEmpty());
        assertFalse(list.remove(100));
        assertTrue(list.remove(20));
        assertTrue(list.isEmpty());
    }

    @Test
    public void add() throws Exception {
        CyclicBarrier barrier = new CyclicBarrier(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable ignored) {
                }
                list.add(finalI);
            }));
            threads.get(i).start();
        }
        for (Thread thread : threads) {
            thread.join();
        }

        for (int i = 0; i < threadCount; i++) {
            assertTrue(list.contains(i));
        }
    }

    @Test
    public void remove() throws Exception {
        for (int i = 0; i < threadCount; i++)
            list.add(i);

        CyclicBarrier barrier = new CyclicBarrier(threadCount);
        AtomicInteger countOfDeleting = new AtomicInteger(0);
        for (int i = 0; i < threadCount; ++i) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable ignored) {
                }
                if (list.remove(finalI))
                    countOfDeleting.incrementAndGet();
            }));
            threads.get(i).start();
        }
        for (Thread thread : threads) {
            thread.join();
        }

        assertEquals(threadCount, countOfDeleting.get());
        for (int i = 0; i < threadCount; i++) {
            assertFalse(list.contains(i));
        }
        assertTrue(list.isEmpty());
    }

    @Test
    public void contains() throws Exception {
        CyclicBarrier barrier = new CyclicBarrier(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable ignored) {
                }
                list.add(finalI);
            }));
            threads.get(i).start();
        }
        for (Thread thread : threads) {
            thread.join();
        }

        assertTrue(list.contains(1023));
        for (int i = 0; i < threadCount - 1; i++)
            assertTrue(list.remove(i));
        assertTrue(list.contains(1023));
        assertTrue(list.remove(1023));
        assertFalse(list.contains(1023));
        assertTrue(list.isEmpty());
    }
}