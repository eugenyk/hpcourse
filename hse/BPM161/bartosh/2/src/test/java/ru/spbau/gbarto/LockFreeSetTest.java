package ru.spbau.gbarto;

import org.junit.Test;

import java.util.Iterator;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    @Test
    public void add() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertTrue(set.add(8));
        assertTrue(set.add(800));
        assertTrue(set.add(555));
        assertTrue(set.add(3535));
        assertFalse(set.add(8));
        assertFalse(set.add(800));
        assertFalse(set.add(555));
        assertFalse(set.add(3535));
    }

    @Test
    public void remove() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        set.add(8);
        set.add(800);

        assertTrue(set.remove(8));
        assertTrue(set.remove(800));
        assertFalse(set.remove(555));
        assertFalse(set.remove(3535));
    }

    @Test
    public void contains() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        set.add(8);
        set.add(800);

        assertTrue(set.contains(8));
        assertTrue(set.contains(800));
        assertFalse(set.contains(555));
        assertFalse(set.contains(3535));
    }

    @Test
    public void isEmpty() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        assertTrue(set.isEmpty());

        set.add(8);
        set.add(800);

        assertFalse(set.isEmpty());

        set.remove(8);
        set.remove(800);

        assertTrue(set.isEmpty());
    }

    @Test
    public void iterator() {
        LockFreeSetInterface<Integer> set = new LockFreeSet<>();

        set.add(8);
        set.add(800);
        set.add(555);
        set.add(3535);

        Iterator<Integer> setIterator = set.iterator();

        assertEquals(8, (int) setIterator.next());
        assertEquals(555, (int) setIterator.next());
        assertEquals(800, (int) setIterator.next());
        assertEquals(3535, (int) setIterator.next());
    }
}