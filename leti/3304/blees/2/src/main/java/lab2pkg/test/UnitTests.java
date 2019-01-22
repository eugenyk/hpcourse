package lab2pkg.test;

import java.util.Arrays;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import lab2pkg.source.LockFreePriorityQueue;
import lab2pkg.source.PriorityQueue;
import org.junit.Test;

public class UnitTests {
    @Test
    public void Operations()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();

        assertEquals(0, q.size());
        assertTrue(q.isEmpty());

        q.offer(1);
        q.offer(5);
        q.offer(8);
        q.offer(2);

        assertEquals(4, q.size());
        assertFalse(q.isEmpty());

        assertEquals(Integer.valueOf(1), q.poll());
        assertEquals(3, q.size());
        assertEquals(Integer.valueOf(2), q.poll());
        assertEquals(2, q.size());
        assertEquals(Integer.valueOf(5), q.poll());
        assertEquals(1, q.size());
        assertEquals(Integer.valueOf(8), q.poll());
        assertEquals(0, q.size());
        assertEquals(null, q.poll());

        assertEquals(0, q.size());
        assertTrue(q.isEmpty());
    }

    @Test
    public void SingleAddPoll()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(2);
        assertEquals(Integer.valueOf(2), q.poll());
    }

    @Test
    public void OfferPollTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.offer(1);
        q.offer(2);
        assertEquals(Integer.valueOf(1), q.poll());
        assertEquals(Integer.valueOf(2), q.poll());
    }

    @Test
    public void AddPeekTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(1);
        q.add(2);
        assertEquals(Integer.valueOf(1), q.peek());
        assertEquals(Integer.valueOf(1), q.peek());
    }

    @Test
    public void RemoveTest()
    {
        PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        q.add(1);
        assertEquals(Integer.valueOf(1), q.remove());
    }
}
