package LockFreePriorityQueue;
import LockFreePriorityQueue.LockFreePriorityQueue;

import java.util.Arrays;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.concurrent.ThreadLocalRandom;

import org.junit.Test;
import org.junit.Assert;
import static org.junit.Assert.assertEquals;
public class LockFreePriorityQueueTest {
	@Test
	public void linearTest(){
		LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();
		assertEquals(0, queue.size());
		queue.offer(3);
		assertEquals(1, queue.size());
		
		queue.offer(5);
		queue.offer(2);
		assertEquals(3,queue.size());
		assertEquals(2, queue.peek().intValue());
		
		queue.offer(4);
		queue.offer(1);
		
		assertEquals(5, queue.size());
		assertEquals(1, queue.peek().intValue());
		
		assertEquals(1, queue.poll().intValue());
		assertEquals(4, queue.size());
		assertEquals(2, queue.poll().intValue());
		
		queue.offer(4);
		queue.offer(0);

        assertEquals(0, queue.poll().intValue());
        assertEquals(4, queue.size());

        assertEquals(3, queue.poll().intValue());
        assertEquals(4, queue.poll().intValue());
        assertEquals(4, queue.poll().intValue());
        assertEquals(5, queue.poll().intValue());
        assertEquals(0, queue.size());

        assertEquals(null, queue.poll());
        assertEquals(null, queue.peek());

        assertEquals(0, queue.size());

        queue.offer(7);
        assertEquals(1, queue.size());
        assertEquals(7, queue.poll().intValue());
        assertEquals(0, queue.size());
		
		
	}
	
	@Test
	public void order(){
		Queue<Integer> lockFreePQ = new LockFreePriorityQueue<Integer>();
		ThreadLocalRandom random = ThreadLocalRandom.current();
		int nElements = random.nextInt(1000,5000);
		int[] ints = random.ints(nElements).toArray();
		
		Arrays.stream(ints).parallel().forEach(lockFreePQ::offer);
		int prev = Integer.MIN_VALUE;
		for (int i = 0; i < nElements; i++) {
			Integer lfpq_value = lockFreePQ.poll();
			Assert.assertTrue(prev<=lfpq_value.intValue());
			prev=lfpq_value.intValue();
			
		}
	}
	
	 @Test
	    public void parallelOffer() {
	        Queue<Integer> lockFreePQ = new LockFreePriorityQueue<>();
	        Queue<Integer> pq = new PriorityQueue<>();

	        ThreadLocalRandom random = ThreadLocalRandom.current();

	        int nElements = random.nextInt(10_000, 50_000);
	        int[] ints = random.ints(nElements).toArray();

	        Arrays.stream(ints)
	                .sequential()
	                .forEach(pq::offer);

	        Arrays.stream(ints)
	                .parallel()
	                .forEach(lockFreePQ::offer);

	        Assert.assertEquals(pq.size(), lockFreePQ.size());

	        for (int i = 0; i < nElements; i++) {
	            Integer pq_value = pq.poll();
	            Integer lfpq_value = lockFreePQ.poll();
	            Assert.assertEquals(pq.size(), lockFreePQ.size());
	            Assert.assertEquals(pq_value, lfpq_value);
	        }
	    }
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

}
