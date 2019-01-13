package ru.ifmo.priorityqueue;

import org.junit.Before;
import org.junit.Test;

import java.util.*;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

import static junit.framework.TestCase.assertTrue;
import static org.assertj.core.api.AssertionsForClassTypes.assertThat;

public class LockFreePriorityQueueTest {
    private LockFreePriorityQueue<Integer> LFPQ;
    private List<Integer> elements;

    private ThreadLocalRandom random = ThreadLocalRandom.current();

    @Before
    public void before() {
        LFPQ = new LockFreePriorityQueue<>();
        final int amount = random.nextInt(1700, 17_000);
        elements = random
                .ints(amount)
                .boxed()
                .collect(Collectors.toList());
    }

    @Test
    public void testOfferAndSize() {
        putElementsToPQ();
    }

    @Test
    public void testOfferAndPollInSeries() {
        putElementsToPQ();
        Integer previous = null;
        Integer current;
        for (int i = 0; i < elements.size(); i++) {
            current = LFPQ.poll();
            if (previous != null) {
                assertThat(current).isGreaterThanOrEqualTo(previous);
            }
            previous = current;
        }
        assertTrue(LFPQ.isEmpty());
    }

    @Test
    public void testOfferAndPollInParallel() {
        final int N = 50;
        ArrayList<Thread> threadsList = new ArrayList<>();
        CyclicBarrier barrier = new CyclicBarrier(N);
        List<Integer> deletedElements = new ArrayList<>();

        AtomicReference<Boolean> allAdded = new AtomicReference<>(true);
        for (int i = 1; i <= N; i++) {
            Integer number = i;
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable ignored) {
                }
                if (allAdded.get() && !LFPQ.offer(number)) {
                    allAdded.set(false);
                }
            }));
        }

        for (int i = N; i > 0; i--) {
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable ignored) {
                }
                Integer number = LFPQ.poll();
                while (number == null) {
                    number = LFPQ.poll();
                }
                deletedElements.add(number);
            }));
        }

        threadsList.forEach(Thread::start);
        threadsList.forEach(t -> {
            try {
                t.join();
            } catch (InterruptedException ignored) {
            }
        });

        assertTrue(allAdded.get());
        assertThat(deletedElements.size()).isEqualTo(N);
        for (int i = 1; i <= N; i++) {
            assertTrue(deletedElements.contains(i));
        }
    }

    private void putElementsToPQ() {
        assertTrue(LFPQ.isEmpty());
        elements.parallelStream().forEach(LFPQ::offer);
        assertThat(LFPQ.size()).isEqualTo(elements.size());
    }
}
