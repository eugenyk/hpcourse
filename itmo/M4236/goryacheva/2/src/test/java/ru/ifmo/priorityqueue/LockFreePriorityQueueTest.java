package ru.ifmo.priorityqueue;

import org.junit.Before;
import org.junit.Test;

import java.util.*;
import java.util.PriorityQueue;
import java.util.stream.Collectors;

import static org.assertj.core.api.AssertionsForClassTypes.assertThat;
import static org.junit.Assert.assertFalse;

public class LockFreePriorityQueueTest {

    private LockFreePriorityQueue<Integer> LFPQ;
    private PriorityQueue<Integer> javaPQ;
    private List<Integer> elements;

    @Before
    public void before() {
        LFPQ = new LockFreePriorityQueue<>();
        javaPQ = new PriorityQueue<>();
        elements = new Random()
                .ints(100, -100, 101)
                .boxed()
                .collect(Collectors.toList());
    }

    @Test
    public void testOfferSimple() {
        putElementsToPQs();
        Iterator<Integer> javaPQiterator = javaPQ.iterator();
        Iterator<Integer> LFPQiterator = LFPQ.iterator();
        while (javaPQiterator.hasNext()) {
            assertThat(LFPQiterator.next()).isEqualTo(javaPQiterator.next());
        }
        assertFalse(LFPQiterator.hasNext());
    }

    @Test
    public void testPollSimple() {
        putElementsToPQs();
        for (int i = 0; i < elements.size(); i++) {
            assertThat(LFPQ.poll()).isEqualTo(javaPQ.poll());
        }
        assertThat(LFPQ.size()).isEqualTo(0);
        assertThat(javaPQ.size()).isEqualTo(0);
    }

    @Test
    public void testRemoveSimple() {
        putElementsToPQs();
        Collections.shuffle(elements);
        for (Integer e : elements) {
            assertThat(LFPQ.remove(e)).isEqualTo(javaPQ.remove(e));
            assertThat(LFPQ.peek()).isEqualTo(javaPQ.peek());
        }
        assertThat(LFPQ.size()).isEqualTo(0);
        assertThat(javaPQ.size()).isEqualTo(0);
    }

    private void putElementsToPQs() {
        assertThat(LFPQ.size()).isEqualTo(0);
        assertThat(javaPQ.size()).isEqualTo(0);
        for (Integer i : elements) {
            LFPQ.offer(i);
            javaPQ.offer(i);
        }
        assertThat(LFPQ.size()).isEqualTo(elements.size());
        assertThat(javaPQ.size()).isEqualTo(elements.size());
    }
}
