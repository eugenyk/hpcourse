package ru.hse.myutman.lockfreeset;

import org.junit.Test;

import java.util.Iterator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class SinglethreadTest {

    @Test
    public void testEmptySet() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
    }

    @Test
    public void testNotEmptySet() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        assertFalse(set.isEmpty());
    }

    @Test
    public void testContains() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        assertTrue(set.contains(6));
    }

    @Test
    public void testNotContains() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        assertFalse(set.contains(5));
    }

    @Test
    public void testShouldRemove() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        set.remove(6);
        assertFalse(set.contains(6));
    }

    @Test
    public void testShouldAddAfterRemove() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        set.remove(6);
        set.add(6);
        assertTrue(set.contains(6));
    }

    @Test
    public void testAddShouldReturnTrue() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(6));
    }

    @Test
    public void testAddShouldReturnFalse() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        assertFalse(set.add(6));
    }

    @Test
    public void testRemoveShouldReturnTrue() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        assertTrue(set.remove(6));
    }

    @Test
    public void testRemoveShouldReturnFalse() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertFalse(set.remove(6));
    }

    @Test
    public void testIterator() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(6);
        set.add(5);
        set.add(4);
        Iterator<Integer> iterator = set.iterator();
        assertEquals(4, (int) iterator.next());
        assertEquals(5, (int) iterator.next());
        assertEquals(6, (int) iterator.next());
    }
}
