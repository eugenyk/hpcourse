package ru.spbau.farutin.lockfreeset;

import org.junit.Before;
import org.junit.Test;

import java.util.Iterator;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    private LockFreeSetInterface<Integer> set;

    @Before
    public void setUp() {
        set = new LockFreeSet<>();
    }

    @Test
    public void testAdd() {
        assertTrue(set.add(1));
        assertTrue(set.contains(1));
        assertTrue(set.add(2));
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
    }

    @Test
    public void testRemove() {
        assertFalse(set.remove(1));
        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertTrue(set.isEmpty());
        assertFalse(set.remove(1));
    }

    @Test
    public void testContains() {
        assertFalse(set.contains(1));
        assertTrue(set.add(1));
        assertTrue(set.contains(1));
    }

    @Test
    public void testIsEmpty() {
        assertTrue(set.isEmpty());
        assertTrue(set.add(1));
        assertFalse(set.isEmpty());
        assertTrue(set.remove(1));
        assertTrue(set.isEmpty());
    }

    @Test
    public void testIterator() {
        assertFalse(set.iterator().hasNext());
        assertTrue(set.add(1));

        Iterator<Integer> iterator = set.iterator();
        assertTrue(iterator.hasNext());
        assertEquals(1, (int)iterator.next());
        assertFalse(iterator.hasNext());

        assertTrue(set.remove(1));
        assertFalse(set.iterator().hasNext());
    }
}
