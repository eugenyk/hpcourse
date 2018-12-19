package org.anstreth.queue;

import org.junit.Assert;
import org.junit.Test;

import static junit.framework.TestCase.assertTrue;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

public class LogicTest {
    private PriorityQueue<Integer> queue = new ConcurrentPriorityQueue<>();

    @Test
    public void queue_correctly_orders_elements() {
        queue.offer(2);
        queue.offer(1);
        queue.offer(3);

        assertEquals(3, (int)queue.poll());
        assertEquals(2, (int)queue.poll());
        assertEquals(1, (int)queue.poll());
    }

    @Test
    public void queue_size_is_appropriate() {
        assertTrue(queue.isEmpty());
        assertEquals(0, queue.size());

        queue.offer(1);
        assertEquals(1, queue.size());

        queue.offer(2);
        assertEquals(2, queue.size());

        queue.poll();
        assertEquals(1, queue.size());
    }

    @Test
    public void peek_shows_top_element() {
        assertNull(queue.peek());

        queue.offer(100);
        assertEquals(100, (int)queue.peek());

        queue.offer(200);
        assertEquals(200, (int)queue.peek());
    }
}
