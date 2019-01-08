package ru.sofysmo.priorityqueue;


import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class LockFreePriorityQueueLogicTest {
    @Test
    public void checkOffer1() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();
        quere.offer(4);
        assertEquals(Integer.valueOf(4), quere.peek());
        quere.offer(3);
        assertEquals(Integer.valueOf(3), quere.peek());
        quere.offer(5);
        assertEquals(Integer.valueOf(3), quere.peek());
        quere.offer(4);
        assertEquals(Integer.valueOf(3), quere.peek());
        quere.offer(-1);
        assertEquals(Integer.valueOf(-1), quere.peek());
    }

    @Test
    public void checkOffer2() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();
        quere.offer(0);
        assertEquals(Integer.valueOf(0), quere.poll());

    }

    @Test
    public void checkPool() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();
        assertEquals(null, quere.poll());
    }

    @Test
    public void checkPeek1() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();
        quere.offer(4);
        assertEquals(Integer.valueOf(4), quere.peek());
        assertEquals(Integer.valueOf(4), quere.poll());
        assertEquals(null, quere.poll());
        assertEquals(null, quere.peek());

        quere.offer(4);
        assertEquals(Integer.valueOf(4), quere.peek());
        assertEquals(Integer.valueOf(4), quere.poll());
        assertEquals(null, quere.peek());
        assertEquals(null, quere.poll());
    }

    @Test
    public void checkPool2() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();

        assertEquals(null, quere.poll());

        quere.offer(4);
        assertEquals(Integer.valueOf(4), quere.poll());
        assertEquals(null, quere.poll());

        quere.offer(3);
        quere.offer(2);
        assertEquals(Integer.valueOf(2), quere.poll());
        assertEquals(Integer.valueOf(3), quere.poll());
        assertEquals(null, quere.poll());
    }

    @Test
    public void checkEmpty() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();

        assertEquals(true, quere.isEmpty());

        quere.offer(4);
        assertEquals(false, quere.isEmpty());
        quere.poll();
        assertEquals(true, quere.isEmpty());

        quere.offer(3);
        quere.offer(2);
        quere.poll();
        assertEquals(false, quere.isEmpty());
        quere.poll();
        assertEquals(true, quere.isEmpty());
        quere.poll();
        assertEquals(true, quere.isEmpty());
    }

    @Test
    public void checkSize() {
        PriorityQueue<Integer> quere = new LockFreePriorityQueue<>();
        assertEquals(0, 0);
        quere.offer(4);
        assertEquals(1, quere.size());
        quere.offer(5);
        assertEquals(2, quere.size());
        quere.poll();
        assertEquals(1, quere.size());
        quere.poll();
        assertEquals(0, quere.size());
        quere.poll();
        assertEquals(0, quere.size());

        quere.offer(4);
        quere.offer(5);
        assertEquals(2, quere.size());
    }
}
