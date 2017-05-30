import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetTest {
    @org.junit.Test
    public void smoke() throws Exception {
        LockFreeSet<Integer> a = new LockFreeSet<Integer>();
        assertTrue(a.isEmpty());
        assertTrue(a.add(1));
        assertFalse(a.add(1));
        assertTrue(a.add(2));
        assertTrue(a.add(3));
        assertTrue(a.contains(1));
        assertTrue(a.remove(1));
        assertFalse(a.remove(1));
        assertFalse(a.contains(1));
        assertFalse(a.isEmpty());
    }
}