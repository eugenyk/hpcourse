package ru.ifmo.priorityqueue;

import org.junit.Before;
import org.junit.Test;

import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
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
    public void testOfferAndPoll() {
        putElementsToPQ();
        Integer previous = null;
        Integer current;
        for (int i = 0; i < elements.size(); i++) {
            current = LFPQ.poll();
            if(previous != null) {
                assertThat(current).isGreaterThanOrEqualTo(previous);
            }
            previous = current;
        }
        assertTrue(LFPQ.isEmpty());
    }

    private void putElementsToPQ() {
        assertTrue(LFPQ.isEmpty());
        elements.parallelStream().forEach(LFPQ::offer);
        assertThat(LFPQ.size()).isEqualTo(elements.size());
    }


}
