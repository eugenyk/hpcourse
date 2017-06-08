import org.junit.Test;
import ru.spbau.mit.LockFreeListSet;
import ru.spbau.mit.LockFreeSet;

import java.util.ArrayList;
import java.util.List;

import static junit.framework.TestCase.assertTrue;
import static org.junit.Assert.assertFalse;

public class LockFreeSetTest {

    @Test
    public void testListSetOneThread() {
        final int n = 100;
        LockFreeSet<Integer> set = new LockFreeListSet<Integer>();
        for (int i = 0; i < n; i++) {
            assertFalse(set.contains(i));
            assertTrue(set.add(i));
            assertFalse(set.add(i));
        }
        assertFalse(set.contains(n));
        assertFalse(set.isEmpty());
        for (int i = 0; i < n; i++) {
            assertTrue(set.contains(i));
            set.remove(i);
            assertFalse(set.contains(i));
        }
        assertTrue(set.isEmpty());
    }

    @Test
    public void testListSetMultiThread() throws InterruptedException {
        final int threadsCnt = 100;
        final int operationsPerThread = 10;

        final LockFreeSet<Integer> set = new LockFreeListSet<Integer>();
        List<Thread> threads = new ArrayList<Thread>();
        for (int i = 0; i < threadsCnt; i++) {
            final int finalI = i;
            threads.add(new Thread(new Runnable() {
                public void run() {
                    for (int operation = 0; operation < operationsPerThread; operation++) {
                        final Integer addValue = finalI + threadsCnt * operation;
                        assertFalse(set.contains(addValue));
                        assertTrue(set.add(addValue));
                        assertFalse(set.add(addValue));
                    }
                }
            }));
            threads.get(i).start();
        }
        for (Thread thread : threads) {
            thread.join();
        }
        assertFalse(set.contains(threadsCnt * operationsPerThread));
        assertFalse(set.isEmpty());

        threads = new ArrayList<Thread>();
        for (int i = 0; i < threadsCnt; i++) {
            final int finalI = i;
            threads.add(new Thread(new Runnable() {
                public void run() {
                    for (int operation = 0; operation < operationsPerThread; operation++) {
                        final Integer removeValue = finalI + threadsCnt * operation;
                        assertTrue(set.contains(removeValue));
                        assertTrue(set.remove(removeValue));
                        assertFalse(set.contains(removeValue));
                    }
                }
            }));
            threads.get(i).start();
        }
        for (Thread thread : threads) {
            thread.join();
        }
        assertTrue(set.isEmpty());
    }
}
