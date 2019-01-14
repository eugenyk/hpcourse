import org.junit.Test;

import java.util.Queue;

import static junit.framework.TestCase.assertTrue;
import static org.junit.Assert.assertEquals;

public class PriorityQueueTests {
    @Test
    public void simpleTest() {
        Queue<Integer> queue = new LockFreePriorityQueue<>();
        assertTrue(queue.offer(1));
        assertEquals(queue.peek(), (Integer)1);
        assertEquals(queue.poll(), (Integer)1);
        assertTrue(queue.isEmpty());

    }

    @Test
    public void priorityTest() {
        Queue<Integer> queue = new LockFreePriorityQueue<>();
        assertTrue(queue.offer(1));
        assertTrue(queue.offer(-2));
        assertTrue(queue.offer(55));
        assertTrue(queue.offer(7));
        assertTrue(queue.offer(90));
        assertTrue(queue.offer(-10));
        assertEquals(queue.poll(), (Integer)(-10));
        assertTrue(!queue.isEmpty());
        assertEquals(queue.poll(), (Integer)(-2));
        assertEquals(queue.poll(), (Integer)1);
        assertEquals(queue.poll(), (Integer)7);
        assertEquals(queue.poll(), (Integer)55);
        assertEquals(queue.poll(), (Integer)90);
        assertTrue(queue.isEmpty());
    }

    @Test
    public void multiThreadOfferPollTestWithoutChecks() throws InterruptedException {
        for (int i = 0; i < 100; i++) {
            Queue<Integer> queue = new LockFreePriorityQueue<>();
            Thread th1 = new Thread(() -> {
                queue.isEmpty();
                assertTrue(queue.offer(5));
                queue.isEmpty();
                queue.isEmpty();
                queue.isEmpty();
            });
            Thread th2 = new Thread(() -> {
                assertTrue(queue.offer(5));
                assertTrue(queue.offer(3));
                queue.poll();
                queue.poll();
                queue.isEmpty();
                assertTrue(queue.offer(4));
            });
            Thread th3 = new Thread(() -> {
                queue.poll();
                queue.isEmpty();
                assertTrue(queue.offer(3));
            });
            th1.start();
            th2.start();
            th3.start();
            th1.join();
            th2.join();
            th3.join();
        }
    }

    @Test
    public void multiThreadOfferPollTests() throws InterruptedException {
        Queue<Integer> queue = new LockFreePriorityQueue<>();
        Thread th1 = new Thread(() -> {
            for (int i = 0; i < 9000; i += 3) assertTrue(queue.offer(i));
        });
        Thread th2 = new Thread(() -> {
            for (int i = 8999; i > 0; i -= 3) assertTrue(queue.offer(i));
        });
        Thread th3 = new Thread(() -> {
            for (int i = 1; i < 9000; i += 3) assertTrue(queue.offer(i));
        });
        th1.start();
        th2.start();
        th3.start();
        th1.join();
        th2.join();
        th3.join();
        for (int i = 0; i < 9000; i++) {
            assertEquals(queue.poll(), (Integer)i);
        }
        assertTrue(queue.isEmpty());
    }
}
