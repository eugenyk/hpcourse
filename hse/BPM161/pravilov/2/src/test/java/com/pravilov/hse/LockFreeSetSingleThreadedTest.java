package com.pravilov.hse;

import org.junit.Test;

import java.util.Iterator;

import static org.junit.Assert.*;

public class LockFreeSetSingleThreadedTest {
    @Test
    public void add() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertTrue(set.add(5));
        assertTrue(set.add(6));
        assertFalse(set.add(5));
        assertTrue(set.add(-1));
        assertFalse(set.add(-1));
        assertFalse(set.add(6));
        assertFalse(set.add(5));
    }

    @Test
    public void remove() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertFalse(set.remove(-1));
        assertFalse(set.remove(5));
        set.add(5);
        assertTrue(set.remove(5));
        assertFalse(set.remove(5));
        set.add(-1);
        set.add(-1);
        assertTrue(set.remove(-1));
        assertFalse(set.remove(6));
        set.add(6);
        set.add(5);
        assertTrue(set.remove(6));
        assertTrue(set.remove(5));
        assertFalse(set.remove(6));
    }

    @Test
    public void contains() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertFalse(set.contains(-1));
        assertFalse(set.contains(5));
        set.add(5);
        assertTrue(set.contains(5));
        set.remove(5);
        assertFalse(set.contains(5));
        set.add(-1);
        assertTrue(set.contains(-1));
        set.remove(1);
        assertTrue(set.contains(-1));
        set.remove(-1);
        assertFalse(set.contains(-1));
        assertFalse(set.contains(5));
        set.add(5);
        assertTrue(set.contains(5));
    }

    @Test
    public void isEmpty() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertTrue(set.isEmpty());
        set.remove(-1);
        assertTrue(set.isEmpty());
        set.remove(5);
        assertTrue(set.isEmpty());
        set.add(5);
        assertFalse(set.isEmpty());
        set.remove(5);
        assertTrue(set.isEmpty());
        set.add(-1);
        assertFalse(set.isEmpty());
        set.remove(-1);
        assertTrue(set.isEmpty());
        set.remove(6);
        assertTrue(set.isEmpty());
        set.add(6);
        assertFalse(set.isEmpty());
        set.add(5);
        assertFalse(set.isEmpty());
        set.remove(6);
        assertFalse(set.isEmpty());
        set.remove(5);
        assertTrue(set.isEmpty());
    }

    @Test
    public void iterator() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();
        Iterator<Integer> setIterator;

        assertFalse(set.iterator().hasNext());
        set.add(5);
        assertTrue(set.iterator().hasNext());
        assertEquals(5, (int) set.iterator().next());
        set.add(6);
        set.remove(5);
        set.add(2);
        setIterator = set.iterator();
        assertEquals(2, (int) setIterator.next());
        assertEquals(6, (int) setIterator.next());
    }

    @Test
    public void iteratorReturnsCopy() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();
        set.add(5);
        set.add(2);
        set.add(12);

        Iterator<Integer> setIterator = set.iterator();

        set.remove(2);
        set.remove(5);
        assertEquals(2, (int) setIterator.next());
        assertEquals(5, (int) setIterator.next());
        assertEquals(12, (int) setIterator.next());
    }
}