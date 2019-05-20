import org.junit.Test;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import static org.junit.Assert.*;

public class LockFreeSetImplTest {

    @Test
    public void add() {
        LockFreeSet<Long> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1L));
        assertTrue(set.add(2L));
        assertTrue(set.add(3L));
        assertFalse(set.add(3L));
        assertSetEquals(set, 1L, 2L, 3L);
    }

    @Test
    public void remove() {
        LockFreeSet<Long> set = new LockFreeSetImpl<>();
        init(set, 1L, 2L, 3L, 4L, 5L, 6L);
        assertTrue(set.remove(4L));
        assertFalse(set.remove(4L));
        assertTrue(set.remove(3L));
        assertTrue(set.remove(5L));
        set.add(3L);
        assertTrue(set.remove(3L));
        assertSetEquals(set, 1L, 2L, 6L);
    }

    @Test
    public void contains() {
        LockFreeSet<Long> set = new LockFreeSetImpl<>();
        init(set, 1L, 2L, 3L);
        assertTrue(set.contains(1L));
        assertTrue(set.contains(2L));
        assertTrue(set.contains(3L));
        assertFalse(set.contains(4L));
        assertFalse(set.contains(5L));
        assertFalse(set.contains(0L));
    }

    @Test
    public void isEmpty() {
        LockFreeSet<Long> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
        init(set, 1L, 2L, 3L);
        assertFalse(set.isEmpty());
        remove(set, 1L, 2L, 3L);
        assertTrue(set.isEmpty());
    }

    @Test
    public void iterator() {
        LockFreeSet<Long> set = new LockFreeSetImpl<>();
        init(set, 1L, 2L, 35L, 4L, 5L);
        Iterator<Long> iterator = set.iterator();
        assertEquals(iterator.next(), (Long) 1L);
        assertEquals(iterator.next(), (Long) 2L);
        assertEquals(iterator.next(), (Long) 35L);
        assertEquals(iterator.next(), (Long) 4L);
        assertEquals(iterator.next(), (Long) 5L);
        assertFalse(iterator.hasNext());
    }

    private <T extends Comparable<T>> void assertSetEquals(LockFreeSet<T> expectedSet, T... correctValues) {
        Set<T> correctSet = new HashSet<>(Arrays.asList(correctValues));
        Set<T> expected = new HashSet<>();
        expectedSet.iterator().forEachRemaining(expected::add);
        assertEquals(correctSet, expected);
    }

    private <T extends Comparable<T>> void init(LockFreeSet<T> set, T... values) {
        for (T value : values) {
            set.add(value);
        }
    }

    private <T extends Comparable<T>> void remove(LockFreeSet<T> set, T... values) {
        for (T value : values) {
            set.remove(value);
        }
    }
}