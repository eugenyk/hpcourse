package ru.spbau.mit;

import org.junit.Test;

import static org.junit.Assert.*;

public class LockFreeListTest {
    private final static int DEFAULT_VALUE = 1;

    @Test
    public void testEmpty() throws Exception {
        LockFreeSet<Integer> ll = new LockFreeSetImpl<>();

        assertTrue(ll.isEmpty());
    }

    @Test
    public void testAdd() throws Exception {
        LockFreeSet<Integer> ll = new LockFreeSetImpl<>();

        ll.append(0);
        ll.append(1);

        assertTrue(ll.contains(0));
        assertTrue(ll.contains(1));
        assertFalse(ll.contains(2));
    }

    @Test
    public void testRemove() throws Exception {
        LockFreeSet<Integer> ll = new LockFreeSetImpl<>();

        ll.append(0);
        ll.append(1);

        assertTrue(ll.remove(0));
        assertFalse(ll.contains(0));

        assertFalse(ll.remove(2));
        assertFalse(ll.contains(2));

        assertFalse(ll.isEmpty());
        assertTrue(ll.remove(1));
        assertTrue(ll.isEmpty());
    }
}
