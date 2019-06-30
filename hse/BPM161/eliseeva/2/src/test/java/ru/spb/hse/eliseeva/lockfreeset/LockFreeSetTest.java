package ru.spb.hse.eliseeva.lockfreeset;

import org.junit.Test;

import java.util.Iterator;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    @Test
    public void addTest() {
        LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
        assertTrue(lockFreeSet.add(1));
        assertFalse(lockFreeSet.add(1));
        assertTrue(lockFreeSet.add(5));
        assertTrue(lockFreeSet.add(-10));
        assertFalse(lockFreeSet.add(5));
    }

    @Test
    public void removeTest() {
        LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
        assertFalse(lockFreeSet.remove(9));
        lockFreeSet.add(1);
        lockFreeSet.add(-3);
        lockFreeSet.add(-2);
        lockFreeSet.add(10);
        lockFreeSet.add(8);
        assertTrue(lockFreeSet.remove(1));
        assertTrue(lockFreeSet.remove(-3));
        assertFalse(lockFreeSet.remove(1234));
        assertTrue(lockFreeSet.remove(-2));
        assertTrue(lockFreeSet.remove(10));
        assertFalse(lockFreeSet.remove(10));

    }

    @Test
    public void containsTest() {
        LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
        lockFreeSet.add(1);
        assertTrue(lockFreeSet.contains(1));
        assertFalse(lockFreeSet.contains(5));
        lockFreeSet.add(25);
        assertTrue(lockFreeSet.contains(25));
        lockFreeSet.add(25);
        assertTrue(lockFreeSet.contains(25));
        lockFreeSet.remove(25);
        assertFalse(lockFreeSet.contains(25));
    }

    @Test
    public void iteratorTest() {
        LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
        lockFreeSet.add(1);
        lockFreeSet.add(25);
        lockFreeSet.add(8);
        lockFreeSet.remove(25);
        lockFreeSet.add(15);
        lockFreeSet.add(7);
        Iterator<Integer> iterator = lockFreeSet.iterator();
        assertTrue(iterator.hasNext());
        assertEquals(1, (int)iterator.next());
        assertTrue(iterator.hasNext());
        assertEquals(7, (int)iterator.next());
        assertTrue(iterator.hasNext());
        assertEquals(8, (int)iterator.next());
        assertTrue(iterator.hasNext());
        assertEquals(15, (int)iterator.next());
        assertFalse(iterator.hasNext());
    }

    @Test
    public void isEmptyTest() {
        LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
        assertTrue(lockFreeSet.isEmpty());
        lockFreeSet.add(1);
        assertFalse(lockFreeSet.isEmpty());
        lockFreeSet.remove(1);
        assertTrue(lockFreeSet.isEmpty());
    }
}
