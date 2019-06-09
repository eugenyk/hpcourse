package hse.kirakosian;

import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class SimpleTest {

    @Test
    public void containsTest() {
        final var set = new LockFreeSet<Integer>();
        assertFalse(set.contains(1));
        set.add(1);
        assertTrue(set.contains(1));
        set.remove(1);
        assertFalse(set.contains(1));
    }

    @Test
    public void deleteTest() {
        final var set = new LockFreeSet<Integer>();
        set.add(1);
        set.remove(1);
        set.remove(0);
        set.add(0);
    }

}
