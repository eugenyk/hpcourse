import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;
import priority_q.LockFreePriorityQueue;

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


}
