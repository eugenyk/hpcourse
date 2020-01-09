import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class SingleThreadTest {

    @Test
    public void isEmptyWorks() {
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        assertTrue(set.contains(1));
        assertFalse(set.contains(0));
    }

    @Test
    public void removeWorks() {
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        assertTrue(set.contains(1));
        set.remove(1);
        assertFalse(set.contains(1));
    }

}
