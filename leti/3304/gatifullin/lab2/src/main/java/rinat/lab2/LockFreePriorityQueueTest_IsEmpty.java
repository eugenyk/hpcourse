package rinat.lab2;

import org.junit.Test;
import threadsafepq.LockFreePriorityQueue;

import static org.junit.Assert.assertEquals;

public class LockFreePriorityQueueTest_IsEmpty extends LockFreePriorityQueueTest {

    @Test
    public void IsEmpty_DefaultQueueWithNoElements_ReturnsTrue() {
        LockFreePriorityQueue<Integer> queue = defaultQueue;
        assertEquals(true, queue.isEmpty());
    }

    @Test
    public void IsEmpty_DefaultQueueWithOneElement_ReturnsFalse() {
        LockFreePriorityQueue<Integer> queue = defaultQueue;
        queue.put(1);
        assertEquals(false, queue.isEmpty());
    }

    @Test
    public void IsEmpty_DefaultQueueWithMultipleElements_ReturnsFalse() {
        LockFreePriorityQueue<Integer> queue = defaultQueue;
        queue.put(1);
        queue.put(2);
        queue.put(3);
        assertEquals(false, queue.isEmpty());
    }

    @Test
    public void IsEmpty_DefaultQueueInsertThenRemove_ReturnsTrue() {
        LockFreePriorityQueue<Integer> queue = defaultQueue;
        queue.put(1);
        queue.remove();
        assertEquals(true, queue.isEmpty());
    }

}