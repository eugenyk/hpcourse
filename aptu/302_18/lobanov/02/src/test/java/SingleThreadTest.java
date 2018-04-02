import org.junit.Test;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import static org.junit.Assert.*;

public class SingleThreadTest {


    @Test
    public void oneElementTest() throws Exception {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
        assertFalse(set.contains(5));
        assertTrue(set.add(5));
        assertTrue(set.contains(5));
        assertFalse(set.isEmpty());
        assertFalse(set.add(5));
        assertTrue(set.remove(5));
        assertFalse(set.contains(5));
        assertTrue(set.isEmpty());
    }

    @Test
    public void severalElementsTest() throws Exception {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final Set<Integer> benchmark = new HashSet<>();
        final int[] elements = {6, 7, -1, 9, 4, 5, 9, 2, -4, 123, 5, -2, 0, 1, 6};
        boolean success = Arrays.stream(elements)
                .allMatch(e -> (benchmark.add(e) == set.add(e)));
        assertTrue(success);

        success = Arrays.stream(elements).allMatch(set::contains);
        assertTrue(success);

        success = Arrays.stream(elements)
                .allMatch(e -> (benchmark.remove(e) == set.remove(e)));
        assertTrue(success);
    }

    @Test
    public void smallRangeRandomTest() {
        final Random random = new Random(239566);
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final Set<Integer> benchmark = new HashSet<>();
        for (int i = 0; i < 1000; i++) {
            final int type = random.nextInt(3);
            final int x = random.nextInt(20);
            if (type == 0) {
                assertEquals(benchmark.add(x), set.add(x));
            } else if (type == 1) {
                assertEquals(benchmark.remove(x), set.remove(x));
            } else {
                assertEquals(benchmark.contains(x), set.contains(x));
            }
        }
    }

    @Test
    public void bigRangeRandomTest() throws Exception {
        final Random random = new Random(239566);
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final Set<Integer> benchmark = new HashSet<>();
        for (int i = 0; i < 100000; i++) {
            final int type = random.nextInt(3);
            final int x = random.nextInt(10000);
            if (type == 0) {
                assertEquals(benchmark.add(x), set.add(x));
            } else if (type == 1) {
                assertEquals(benchmark.remove(x), set.remove(x));
            } else {
                assertEquals(benchmark.contains(x), set.contains(x));
            }
        }
    }

    @Test
    public void isEmpty() throws Exception {
    }

}