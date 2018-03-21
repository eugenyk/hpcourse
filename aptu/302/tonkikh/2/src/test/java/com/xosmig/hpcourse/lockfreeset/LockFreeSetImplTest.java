package com.xosmig.hpcourse.lockfreeset;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.*;

public class LockFreeSetImplTest {
    private final LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Test
    public void returnValueSemantic() throws Exception {
        assertTrue(set.isEmpty());

        assertTrue(set.add(1));
        assertFalse(set.add(1));

        assertFalse(set.isEmpty());

        assertTrue(set.contains(1));
        assertFalse(set.contains(2));

        assertTrue(set.remove(1));
        assertFalse(set.remove(2));
    }

    @Test
    public void simpleAddTest() throws Exception {
        assertTrue(set.add(9));
        assertTrue(set.add(2));
        assertFalse(set.add(9));
    }

    @Test
    public void testConcurrentAddAndRemove() throws Exception {
        final AtomicBoolean stop = new AtomicBoolean(false);
        final int threadsNumber = 10;
        final int maxValue = 10;
        final CountDownLatch start = new CountDownLatch(1);

        final AtomicInteger removes = new AtomicInteger(0);
        Runnable remover = () -> {
            Random random = new Random();
            try {
                start.await();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            while (!stop.get()) {
                if (set.remove(random.nextInt(maxValue))) {
                    removes.incrementAndGet();
                }
            }
        };

        final AtomicInteger adds = new AtomicInteger(0);
        Runnable adder = () -> {
            Random random = new Random();
            try {
                start.await();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            while (!stop.get()) {
                if (set.add(random.nextInt(maxValue))) {
                    adds.incrementAndGet();
                }
            }
        };

        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < threadsNumber; i++) {
            Thread adderThread = new Thread(adder);
            adderThread.start();
            threads.add(adderThread);

            Thread removerThread = new Thread(remover);
            removerThread.start();
            threads.add(removerThread);
        }

        start.countDown();
        Thread.sleep(500);
        stop.set(true);

        for (Thread thread : threads) {
            thread.join();
        }

        int left = 0;
        for (int i = 0; i < maxValue; i++) {
            if (set.contains(i)) {
                left++;
            }
        }

        assertThat(adds.get(), equalTo(removes.get() + left));
    }
}
