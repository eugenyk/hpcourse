package ru.hse.spb.sharkova.lockfreeset;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    @Test
    public void testAdd() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertTrue(set.add(30));
        assertFalse(set.add(30));
    }

    @Test
    public void testContains() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertFalse(set.contains(30));
        set.add(30);
        assertTrue(set.contains(30));
        assertFalse(set.contains(31));
    }

    @Test
    public void testRemove() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertFalse(set.remove(30));
        set.add(30);
        assertTrue(set.remove(30));
        assertFalse(set.contains(30));
        assertFalse(set.remove(30));
    }

    @Test
    public void testIsEmpty() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertTrue(set.isEmpty());
        set.add(30);
        set.add(42);
        assertFalse(set.isEmpty());
        set.remove(30);
        assertFalse(set.isEmpty());
        set.remove(42);
        assertTrue(set.isEmpty());
    }

    @Test
    public void testIterator() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        Iterator<Integer> iterator = set.iterator();
        assertFalse(iterator.hasNext());

        set.add(30);
        set.add(1);
        assertTrue(set.contains(1));
        set.add(42);
        set.add(0);
        assertTrue(set.contains(0));
        set.add(-1);
        assertTrue(set.contains(-1));
        set.add(30);

        List<Integer> values = new ArrayList<>();
        for (iterator = set.iterator(); iterator.hasNext(); ) {
            values.add(iterator.next());
        }
        assertEquals(Arrays.asList(-1, 0, 1, 30, 42), values);
    }
}