package task;

import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;


public class ThreadSafeSetCommonTests {

    @Test
    public void testIsEmpty() {
        ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();

        assertTrue(set.isEmpty());

        set.add(42);

        assertFalse(set.isEmpty());
    }

    @Test
    public void testIsContains() {
        ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();

        assertFalse(set.contains(42));

        set.add(42);

        assertTrue(set.contains(42));

        for (int i = 0; i < 10; ++i) {
            set.add(i);
        }

        for (int i = 0; i < 10; ++i) {
            assertTrue(set.contains(i));
        }

        set.remove(42);
        assertFalse(set.contains(42));
    }
}
