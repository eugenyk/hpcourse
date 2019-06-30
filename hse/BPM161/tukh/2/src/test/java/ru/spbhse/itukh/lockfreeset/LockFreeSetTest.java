package ru.spbhse.itukh.lockfreeset;

import org.junit.Test;

import java.util.Iterator;
import java.util.TreeSet;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    private LockFreeSet<Integer> prepareSet() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        set.add(2);
        set.add(3);
        set.add(9);
        return set;
    }

    @Test
    public void onlyAddingTest() {
        LockFreeSet<Integer> set = prepareSet();
        assertTrue(set.contains(2));
        assertTrue(set.contains(3));
        assertTrue(set.contains(9));
        assertFalse(set.contains(10));
        assertTrue(set.add(10));
        assertFalse(set.add(10));
        assertTrue(set.contains(10));
    }

    @Test
    public void addAndRemoveTest() {
        LockFreeSet<Integer> set = prepareSet();
        set.remove(2);
        assertFalse(set.contains(2));
        set.add(2);
        assertTrue(set.contains(2));
        set.add(2);
        assertTrue(set.contains(2));
        set.remove(10);
        assertFalse(set.contains(10));
        set.add(10);
        assertTrue(set.contains(10));
        assertTrue(set.remove(2));
        assertFalse(set.remove(2));
        assertFalse(set.remove(11));
    }

    @Test
    public void containsTest() {
        LockFreeSet<Integer> set = prepareSet();
        assertTrue(set.contains(2));
        assertTrue(set.contains(3));
        assertTrue(set.contains(9));
        set.remove(2);
        set.remove(3);
        set.remove(9);
        assertFalse(set.contains(2));
        assertFalse(set.contains(3));
        assertFalse(set.contains(9));
    }

    @Test
    public void isEmptyTest() {
        LockFreeSet<Integer> set = prepareSet();
        assertFalse(set.isEmpty());
        set.remove(2);
        assertFalse(set.isEmpty());
        set.remove(3);
        assertFalse(set.isEmpty());
        set.remove(9);
        assertTrue(set.isEmpty());
        set.add(9);
        assertFalse(set.isEmpty());
    }

    @Test
    public void iterator() {
        LockFreeSet<Integer> set = prepareSet();
        Iterator<Integer> iterator = set.iterator();
        TreeSet<Integer> values = new TreeSet<>();
        values.add(2);
        values.add(3);
        values.add(9);
        int count = 0;
        for (; iterator.hasNext(); count++) {
            assertTrue(values.contains(iterator.next()));
        }
        assertEquals(count, values.size());
        set.remove(3);
        set.add(566);
        values.remove(3);
        values.add(566);
        count = 0;
        for (; iterator.hasNext(); count++) {
            assertTrue(values.contains(iterator.next()));
        }
    }

    @Test
    public void complexTest() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertTrue(set.add(0));
        assertFalse(set.contains(1));
        assertFalse(set.isEmpty());
        assertFalse(set.contains(4));
        assertTrue(set.add(2));
        assertTrue(set.add(-10));
        assertFalse(set.isEmpty());
        assertFalse(set.isEmpty());
        assertFalse(set.contains(4));
        assertFalse(set.isEmpty());
        assertTrue(set.add(10));
        assertFalse(set.contains(4));
        assertFalse(set.contains(-9));
    }
}