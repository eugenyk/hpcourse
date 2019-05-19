package hse.kirilenko.lockfreeset;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    @Test
    public void testAddEmpty() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertTrue(lfs.isEmpty());
        assertTrue(lfs.add(1));
        assertFalse(lfs.isEmpty());
    }

    @Test
    public void testAddContains() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertFalse(lfs.contains(1));
        assertFalse(lfs.contains(2));
        assertTrue(lfs.add(1));
        assertTrue(lfs.contains(1));
        assertFalse(lfs.contains(2));
    }

    @Test
    public void testAdd() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertTrue(lfs.add(1));
        assertFalse(lfs.add(1));
    }

    @Test
    public void testRemove() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertFalse(lfs.remove(1));
    }

    @Test
    public void testAddRemove() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertTrue(lfs.add(1));
        assertFalse(lfs.add(1));

        assertTrue(lfs.remove(1));
        assertFalse(lfs.remove(1));
    }

    @Test
    public void testAddRemoveEmpty() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertTrue(lfs.isEmpty());
        assertTrue(lfs.add(1));
        assertFalse(lfs.add(1));
        assertFalse(lfs.isEmpty());
        assertTrue(lfs.remove(1));
        assertFalse(lfs.remove(1));
        assertTrue(lfs.isEmpty());
    }

    @Test
    public void testAddRemoveEmptyContains() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertFalse(lfs.contains(1));
        assertTrue(lfs.isEmpty());
        assertTrue(lfs.add(1));
        assertTrue(lfs.contains(1));
        assertFalse(lfs.add(1));
        assertFalse(lfs.isEmpty());
        assertTrue(lfs.remove(1));
        assertFalse(lfs.contains(1));
        assertFalse(lfs.remove(1));
        assertTrue(lfs.isEmpty());
        assertFalse(lfs.contains(1));
    }

    @Test
    public void testIterator() {
        List<Integer> expected = new ArrayList<>();
        expected.add(1);
        expected.add(2);
        expected.add(3);
        expected.add(4);
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        lfs.add(1);
        lfs.add(2);
        lfs.add(3);
        lfs.add(4);

        List<Integer> actual = new ArrayList<>();

        Iterator<Integer> it = lfs.iterator();

        while (it.hasNext()) {
            actual.add(it.next());
        }

        assertEquals(expected, actual);
    }

    @Test
    public void testIteratorEmpty() {
        LockFreeSet<Integer> lfs = new LockFreeSet<>();
        assertFalse(lfs.iterator().hasNext());
    }
}
