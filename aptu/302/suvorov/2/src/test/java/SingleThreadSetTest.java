import org.junit.Test;

import static org.junit.Assert.*;

public class SingleThreadSetTest {
    private final LockFreeSet<Integer> s;

    public SingleThreadSetTest() {
        s = new LockFreeSetImpl<>();
    }

    @Test
    public void testInitiallyEmpty() {
        assertTrue(s.isEmpty());
        assertFalse(s.contains(123));
    }

    @Test
    public void testAdd() {
        assertTrue(s.add(123));

        assertTrue(s.contains(123));
        assertFalse(s.isEmpty());
    }

    @Test
    public void testMultiAddContains() {
        assertTrue(s.add(123));
        assertFalse(s.add(123));
        assertFalse(s.add(123));

        assertTrue(s.contains(123));
        assertFalse(s.isEmpty());
    }

    @Test
    public void testAddRemoveContains() {
        assertTrue(s.add(123));

        assertTrue(s.remove(123));

        assertFalse(s.contains(123));
        assertTrue(s.isEmpty());
    }

    @Test
    public void testMultiAddRemoveContains() {
        assertTrue(s.add(123));
        assertFalse(s.add(123));

        assertTrue(s.remove(123));

        assertFalse(s.contains(123));
        assertTrue(s.isEmpty());
    }

    @Test
    public void testMultiAddMultiRemoveContains() {
        assertTrue(s.add(123));
        assertFalse(s.add(123));

        assertTrue(s.remove(123));
        assertFalse(s.contains(123));
        assertTrue(s.isEmpty());

        assertFalse(s.remove(123));
        assertFalse(s.contains(123));
        assertTrue(s.isEmpty());
    }

    @Test
    public void testStress() {
        new StressTestUtil(StressTestUtil.DO_ALL, true)
                .stressTest(s, 0, 1000, 100000, 123);
    }
}
