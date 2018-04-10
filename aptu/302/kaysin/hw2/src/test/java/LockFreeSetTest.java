import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetTest {
    @Test
    public void simpleOneThreadAdd() throws Exception {
        LockFreeSetImpl<Integer> s = new LockFreeSetImpl<>();
        assertTrue(s.add(4));
        assertTrue(s.contains(4));
        assertFalse(s.contains(5));

        assertFalse(s.add(4));

        assertTrue(s.add(5));
        assertTrue(s.contains(4));
        assertTrue(s.contains(5));
        assertFalse(s.contains(6));
    }

    @Test
    public void simpleOneThreadRemove() throws Exception {
        LockFreeSetImpl<Integer> s = new LockFreeSetImpl<>();
        s.add(4);
        s.add(5);
        assertTrue(s.remove(5));
        assertFalse(s.contains(5));
        assertFalse(s.remove(5));

        assertTrue(s.contains(4));
        assertFalse(s.contains(5));
    }

    @Test
    public void simpleConcurrentDeadlockTest() throws Exception {
        LockFreeSet<Integer> s = new LockFreeSetImpl<>();

        Thread threadProducer = new Thread(() -> {
           for (int i = 0; i < 20000; i++) {
               s.add(i);
           }
        });

        Thread threadConsumer = new Thread(() -> {
            for (int i = 0; i < 10000; i++) {
                s.remove(i);
            }
        });

        Thread threadConsumer2 = new Thread(() -> {
            for (int i = 0; i < 20000; i++) {
                s.remove(i);
            }
        });

        threadProducer.start();
        threadConsumer2.start();
        threadConsumer.start();

        threadProducer.join();
        threadConsumer2.join();
        threadConsumer.join();
    }
}