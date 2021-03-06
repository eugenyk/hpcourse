package lockFreePriorityQueue;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import org.junit.Test;

public class BasicTest {

	@Test
	public void testSingleThreadOperations() {
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		
		assertTrue(q.isEmpty());
		assertEquals(0, q.size());
		
		q.addAll(Arrays.asList(1, 4, 1, 3, 2));
		
		assertFalse(q.isEmpty());
		assertEquals(5, q.size());
		assertEquals(Integer.valueOf(1), q.peek());
		assertEquals(5, q.size());
		
		assertEquals(Integer.valueOf(1), q.poll());
		assertEquals(Integer.valueOf(1), q.poll());
		assertEquals(Integer.valueOf(2), q.poll());
		assertEquals(Integer.valueOf(3), q.poll());
		
		assertEquals(1, q.size());
		assertFalse(q.isEmpty());
		
		assertEquals(Integer.valueOf(4), q.poll());
		
		assertEquals(null, q.poll());
		assertEquals(null, q.peek());
		
		assertTrue(q.isEmpty());
		assertEquals(0, q.size());
	}
	
	@Test
	public void testSecondInsert() {
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.add(2);
		q.add(1);
		assertEquals(Integer.valueOf(1), q.poll());
		assertEquals(Integer.valueOf(2), q.poll());
	}

	@Test
	public void testDeadlock() {
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.add(3);
		q.add(4);
		q.add(1);
	}
}
