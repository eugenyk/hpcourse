package ru.ifmo.ct.khalansky.hpcourse.queue;
import org.junit.Test;
import static org.junit.Assert.*;
import java.util.function.Function;
import java.util.function.Supplier;

public class SmokeTests {

    @Test
    public void testSingleThreaded() {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
        assert(queue.isEmpty());
        assertEquals(0, queue.size());
        assert(queue.offer(5));
        assert(!queue.isEmpty());
        assertEquals(5, (int)queue.peek());
        assertEquals(1, queue.size());
        assert(queue.offer(3));
        assertEquals(2, queue.size());
        assertEquals(3, (int)queue.peek());
        assertEquals(2, queue.size());
        assert(queue.offer(8));
        assertEquals(3, queue.size());
        assertEquals(3, (int)queue.peek());
        assertEquals(3, queue.size());
        assert(queue.offer(10));
        assert(!queue.isEmpty());
        assertEquals(4, queue.size());
        assertEquals(3, (int)queue.peek());
        assertEquals(4, queue.size());
        assertEquals(3, (int)queue.poll());
        assertEquals(3, queue.size());
        assertEquals(5, (int)queue.peek());
        assertEquals(3, queue.size());
        assertEquals(5, (int)queue.poll());
        assertEquals(2, queue.size());
        assertEquals(8, (int)queue.peek());
        assertEquals(2, queue.size());
        assert(queue.offer(1));
        assertEquals(3, queue.size());
        assertEquals(1, (int)queue.peek());
        assertEquals(1, (int)queue.poll());
        assertEquals(8, (int)queue.poll());
        assertEquals(10, (int)queue.poll());
        assert(queue.isEmpty());
    }

}

