import org.junit.Test;

import java.util.*;

import static org.junit.Assert.assertEquals;

public class LockFreePriorityQueueTest {
    @Test
    public void offerAndOrderTest() {
        LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
        List<Integer> integerList = Arrays.asList(5, 4, 3, 2, 1);
        for (Integer i : integerList) {
            queue.offer(i);
        }
        assertEquals(queue.size(), integerList.size());
        Collections.reverse(integerList);
        for (Integer i : integerList) {
            assertEquals(i, queue.poll());
        }
    }
}
