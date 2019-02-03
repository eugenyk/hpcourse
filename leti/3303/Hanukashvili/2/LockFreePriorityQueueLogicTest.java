package lab2;

import static org.junit.Assert.*;

import org.junit.Test;

public class LockFreePriorityQueueLogicTest {


	@Test
	public void test() {
		PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
        queue.offer(10);
        assertEquals(Integer.valueOf(10), queue.peek());
        queue.offer(20);
        assertEquals(Integer.valueOf(10), queue.peek());
        queue.offer(5);
        assertEquals(Integer.valueOf(5), queue.peek());
	}
	
	@Test
    public void poll_peekTest() {
        PriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();
        assertEquals(null, queue.poll());
        
        queue.offer(4);
        assertEquals(Integer.valueOf(4), queue.peek());
        assertEquals(Integer.valueOf(4), queue.poll());
        assertEquals(null, queue.poll());
        assertEquals(null, queue.peek());
        
        queue.offer(7);
        queue.offer(5);
        queue.offer(10);
        assertEquals(Integer.valueOf(5), queue.peek());
        assertEquals(Integer.valueOf(5), queue.poll());
        assertEquals(Integer.valueOf(7), queue.poll());
    }


    @Test
    public void isEmptyTest() {
        PriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();

        assertEquals(true, queue.isEmpty());
        
        queue.offer(10);
        assertEquals(false, queue.isEmpty());
        queue.poll();
        assertEquals(true, queue.isEmpty());
        
        queue.offer(5);
        queue.offer(7);
        queue.offer(10);
        assertEquals(false, queue.isEmpty());
    }

    @Test
    public void sizeTest() {
        PriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();
        assertEquals(0, 0);
        
        queue.offer(10);
        assertEquals(1, queue.size());
        queue.poll();
        assertEquals(0, queue.size());
        
        queue.offer(5);
        queue.offer(7);
        queue.offer(10);
        
    }

}
