package ifmo.shemetova.priorityq;
import org.junit.After;
import org.junit.Test;


import java.util.concurrent.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class QueueTest {
    ExecutorService executor = Executors.newFixedThreadPool(50);

    @Test
    public void oneThreadTest() {
        LockFreePriorityQueue queue = new LockFreePriorityQueue<Integer>();
        assertTrue(queue.isEmpty());
        assertTrue(queue.offer(2));
        assertFalse(queue.isEmpty());
        assertEquals(1, queue.size());
        assertEquals(2, queue.peek());
        assertTrue(queue.offer(3));
        assertEquals(2, queue.size());
        assertEquals(2, queue.peek());
        assertTrue(queue.offer(1));
        assertEquals(1, queue.peek());
        assertEquals(1, queue.poll());
        assertEquals(2, queue.size());
        assertEquals(2, queue.peek());
    }

    @Test
    public void addManyTest() {
        LockFreePriorityQueue queue = new LockFreePriorityQueue<Integer>();
        CountDownLatch latch = new CountDownLatch(1000);
        for (int i = 0; i < 1000; i++) {
            final int j = i;
            executor.execute(()-> {
                try {
                    queue.offer(j);
                    latch.countDown();
                    TimeUnit.MILLISECONDS.sleep(5);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            });
        }
        try {
            latch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        assertFalse(queue.isEmpty());
        assertEquals(1000, queue.size());
        assertEquals(0, queue.poll());
        assertEquals(1, queue.peek());
    }

    @Test
    public void addAndDeleteTest() {
        LockFreePriorityQueue queue = new LockFreePriorityQueue<Integer>();
        CountDownLatch latch = new CountDownLatch(1500);
        for (int i = 1000; i > 0; i--) {
            final int j = i;
            executor.execute(()-> {
                try {
                    queue.offer(j);
                    TimeUnit.MILLISECONDS.sleep(15);
                    latch.countDown();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            });
        }
        for (int i = 0; i < 500; i++) {
            executor.execute(()-> {
                try {
                    queue.poll();
                    TimeUnit.MILLISECONDS.sleep(15);
                    latch.countDown();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            });
        }
        try {
            latch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        assertFalse(queue.isEmpty());
        assertEquals(500, queue.size());
        assertEquals(501, queue.peek());

    }


    @After
    public void tearDown() {
        executor.shutdown();
    }
}
