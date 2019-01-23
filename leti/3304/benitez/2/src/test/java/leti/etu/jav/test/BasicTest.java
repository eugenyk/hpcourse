package leti.etu.jav.test;

import leti.etu.jav.queue.LockFreePriorityQueue;
import leti.etu.jav.queue.PriorityQueue;
import java.util.Arrays;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

public class BasicTest {

	@Test
	public void singleThreadOps()
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
	public void singleThreadSingleAddPoll()
	{
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.add(2);
		assertEquals(Integer.valueOf(2), q.poll());
	}
	
	@Test
	public void singleThreadElementTest()
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
	public void singleThreadOfferPollTest()
	{
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.offer(1);
		q.offer(2);
		assertEquals(Integer.valueOf(1), q.poll());
		assertEquals(Integer.valueOf(2), q.poll());
	}
	
	@Test
	public void singleThreadAddPeekTest()
	{
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.add(1);
		q.add(2);
		assertEquals(Integer.valueOf(1), q.peek());
		assertEquals(Integer.valueOf(1), q.peek());
	}
	
	@Test
	public void singleThreadRemoveTest()
	{
		PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
		q.add(1);
		assertEquals(Integer.valueOf(1), q.remove());
	}

}
