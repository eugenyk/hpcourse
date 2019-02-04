package test;

import lab2.LockFreePriorityQueue;
import lab2.PriorityQueue;

import java.util.Arrays;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class MainTest {

    @Test
    public void labOps()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();

        assertEquals(0, q.size());
        assertTrue(q.isEmpty());

        q.addAll(Arrays.asList(1, 2, 3, 1));

        assertEquals(4, q.size());
        assertFalse(q.isEmpty());

        assertEquals(Integer.valueOf(1), q.poll());
        assertEquals(3, q.size());
        assertEquals(Integer.valueOf(1), q.poll());
        assertEquals(2, q.size());
        assertEquals(Integer.valueOf(2), q.poll());
        assertEquals(1, q.size());
        assertEquals(Integer.valueOf(3), q.poll());
        assertEquals(0, q.size());
        assertEquals(null, q.poll());

        assertEquals(0, q.size());
        assertTrue(q.isEmpty());
    }

    @Test
    public void labSingleAddPoll()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(2);
        assertEquals(Integer.valueOf(2), q.poll());
    }

    @Test
    public void labElementTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.addAll(Arrays.asList(1, 2, 3, 1));
        assertEquals(Integer.valueOf(1), q.element());
        assertEquals(Integer.valueOf(1), q.element());
        assertEquals(Integer.valueOf(1), q.element());
        assertEquals(Integer.valueOf(1), q.element());
        assertEquals(4, q.size());
    }

    @Test
    public void labOfferPollTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.offer(1);
        q.offer(2);
        assertEquals(Integer.valueOf(1), q.poll());
        assertEquals(Integer.valueOf(2), q.poll());
    }

    @Test
    public void labAddPeekTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(1);
        q.add(2);
        assertEquals(Integer.valueOf(1), q.peek());
        assertEquals(Integer.valueOf(1), q.peek());
    }

    @Test
    public void labRemoveTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(1);
        assertEquals(Integer.valueOf(1), q.remove());
    }

}
