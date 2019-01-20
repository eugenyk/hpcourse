package com.company.test;

import com.company.*;
import java.util.Arrays;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class Test_1 {

    @Test
    public void singleThreadOps()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();

        assertEquals(0, queue.size());
        assertTrue(queue.isEmpty());

        queue.addAll(Arrays.asList(1, 2, 3, 1));

        assertEquals(4, queue.size());
        assertFalse(queue.isEmpty());

        assertEquals(Integer.valueOf(1), queue.poll());
        assertEquals(3, queue.size());
        assertEquals(Integer.valueOf(1), queue.poll());
        assertEquals(2, queue.size());
        assertEquals(Integer.valueOf(2), queue.poll());
        assertEquals(1, queue.size());
        assertEquals(Integer.valueOf(3), queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        assertEquals(0, queue.size());
        assertTrue(queue.isEmpty());
    }

    @Test
    public void singleThreadSingleAddPoll()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();
        queue.offer(2);
        assertEquals(Integer.valueOf(2), queue.poll());
    }

    @Test
    public void singleThreadElementTest()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();
        queue.addAll(Arrays.asList(1, 2, 3, 1));
        assertEquals(Integer.valueOf(1), queue.peek());
        assertEquals(Integer.valueOf(1), queue.peek());
        assertEquals(Integer.valueOf(1), queue.peek());
        assertEquals(Integer.valueOf(1), queue.peek());
        assertEquals(4, queue.size());
    }

    @Test
    public void singleThreadOfferPollTest()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();
        queue.offer(1);
        queue.offer(2);
        assertEquals(Integer.valueOf(1), queue.poll());
        assertEquals(Integer.valueOf(2), queue.poll());
    }

    @Test
    public void singleThreadAddPeekTest()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();
        queue.offer(1);
        queue.offer(2);
        assertEquals(Integer.valueOf(1), queue.peek());
        assertEquals(Integer.valueOf(1), queue.peek());
    }

    @Test
    public void singleThreadRemoveTest()
    {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityLFQueue<>();
        queue.offer(1);
        assertEquals(Integer.valueOf(1), queue.poll());
    }

}