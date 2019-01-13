import org.junit.Assert;
import org.junit.Test;
import priority_q.LockFreePriorityQueue;

import java.util.Arrays;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.concurrent.ThreadLocalRandom;

import static org.junit.Assert.assertEquals;

public class LockFreePriorityQueueTest {
    @Test
    public void linearTest() {
        LockFreePriorityQueue<Integer> q = new LockFreePriorityQueue<>();
        assertEquals(0, q.size());

        q.offer(3);
        assertEquals(1, q.size());

        q.offer(5);
        q.offer(2);
        assertEquals(3, q.size());
        assertEquals(2, q.peek().intValue());

        q.offer(4);
        q.offer(1);

        assertEquals(5, q.size());
        assertEquals(1, q.peek().intValue());

        // --------- pool ---------
        assertEquals(1, q.poll().intValue());
        assertEquals(4, q.size());
        assertEquals(2, q.poll().intValue());

        q.offer(4);
        q.offer(0);

        assertEquals(0, q.poll().intValue());
        assertEquals(4, q.size());

        assertEquals(3, q.poll().intValue());
        assertEquals(4, q.poll().intValue());
        assertEquals(4, q.poll().intValue());
        assertEquals(5, q.poll().intValue());
        assertEquals(0, q.size());

        assertEquals(null, q.poll());
        assertEquals(null, q.peek());

        assertEquals(0, q.size());

        q.offer(7);
        assertEquals(1, q.size());
        assertEquals(7, q.poll().intValue());
        assertEquals(0, q.size());

    }

    @Test
    public void order() {
        Queue<Integer> lockFreePQ = new LockFreePriorityQueue<>();

        ThreadLocalRandom random = ThreadLocalRandom.current();

        int nElements = random.nextInt(10_000, 50_000);
        int[] ints = random.ints(nElements).toArray();

        Arrays.stream(ints)
                .parallel()
                .forEach(lockFreePQ::offer);


        Assert.assertEquals(nElements, lockFreePQ.size());
        int prev = Integer.MIN_VALUE;
        for (int i = 0; i < nElements; i++) {
            Integer lfpq_value = lockFreePQ.poll();

            Assert.assertTrue(prev <= lfpq_value.intValue());
            prev = lfpq_value.intValue();
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
