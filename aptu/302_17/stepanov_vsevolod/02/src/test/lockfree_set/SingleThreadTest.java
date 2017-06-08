package lockfree_set;

import junit.framework.TestCase;
import org.junit.Test;

import java.util.List;
import java.util.Random;
import java.util.stream.Collectors;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class SingleThreadTest {
    private final Random random = new Random(239);

    @Test
    public void testSingleThread() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        List<Integer> list = random.ints().limit(5).boxed().collect(Collectors.toList());
        assertTrue(set.isEmpty());
        for (int i: list) {
            assertTrue(set.add(i));
            System.out.println(i);
        }
        assertFalse(set.isEmpty());
        for (int i: list) {
            assertTrue(set.contains(i));
            System.out.println(i);
        }
        for (int i: list) {
            assertTrue(set.remove(i));
            System.out.println(i);
        }
        assertTrue(set.isEmpty());
        for (int i: list) {
            assertFalse(set.remove(i));
            System.out.println(i);
        }
    }
}