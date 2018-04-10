import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest {

    private LockFreeSet<Integer> set;

    @Before
    public void setup() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void emptyTest() {
        assertTrue(set.isEmpty());
    }

    @Test
    public void addTest() {
        assertTrue(set.add(1));
        assertFalse(set.add(1));
        assertTrue(set.add(0));
        assertTrue(set.add(2));
        assertFalse(set.add(0));
    }

    @Test
    public void removeTest() {
        assertFalse(set.remove(0));
        assertFalse(set.remove(1));
        assertFalse(set.remove(1));
        assertTrue(set.add(0));
        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertFalse(set.isEmpty());
        assertFalse(set.remove(2));
        assertFalse(set.remove(1));
        assertTrue(set.remove(0));
        assertTrue(set.isEmpty());
    }

    @Test
    public void containsTest() {
        assertFalse(set.contains(0));
        assertTrue(set.add(0));
        assertTrue(set.contains(0));
        assertFalse(set.contains(1));
        set.remove(0);
        assertFalse(set.contains(0));
    }

    @Test
    public void multipleThreadsTest() throws InterruptedException {
        List<Thread> threads = new ArrayList<>();
        for (int n = 0; n < 1000; n += 10) {
            int threadId = n;
            threads.add(new Thread(() -> {
                List<Integer> vals = new ArrayList<>();
                for (int i = 0; i < 10; i++) {
                    vals.add(threadId + i);
                }
                for (Integer value: vals) {
                    assertTrue(set.add(value));
                    assertFalse(set.add(value));
                }
                for (Integer value: vals) {
                    assertTrue(set.contains(value));
                }
                for (Integer value: vals) {
                    assertTrue(set.remove(value));
                    assertFalse(set.remove(value));
                }
            }));
        }
        for (Thread thread : threads) {
            thread.start();
        }
        for (Thread thread : threads) {
            thread.join();
        }
        assertTrue(set.isEmpty());
    }
}