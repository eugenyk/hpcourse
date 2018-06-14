import org.junit.Before;
import org.junit.Test;

import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;

public class SingleThreadTest {

    private LockFreeSet<Integer> set;

    @Before
    public void setup() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void testInitialEmpty() {
        assertTrue(set.isEmpty());
    }

    @Test
    public void testRemoveFromEmpty() {
        assertFalse(set.remove(0));
        assertFalse(set.remove(1));
    }

    @Test
    public void testAdd() {
        assertTrue(set.add(0));
        assertFalse(set.add(0));
        assertTrue(set.add(1));
        assertFalse(set.add(1));
    }

    @Test
    public void testContains() {
        assertFalse(set.contains(0));
        set.add(0);
        assertTrue(set.contains(0));
        set.remove(0);
        assertFalse(set.contains(0));
    }

    @Test
    public void testRemove() {
        assertTrue(set.add(0));
        assertTrue(set.remove(0));
        assertFalse(set.remove(0));
    }
}
