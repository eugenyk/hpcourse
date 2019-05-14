package ru.spbau.lockfree;

import org.junit.Test;

import java.util.Iterator;
import java.util.List;

import static org.junit.Assert.*;

public class LockFreeSetSingleThreadTest {

    @Test
    public void add() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertTrue(set.add(0));
        assertTrue(set.add(2));
        assertTrue(set.add(1));
        assertTrue(set.add(-1));
        List<Integer> elements = set.scan();
        assertArrayEquals(new int[]{-1, 0, 1, 2}, elements.stream().mapToInt
                (x -> x).toArray());
    }

    @Test
    public void remove() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        set.add(0);
        set.add(2);
        set.add(1);
        set.add(-1);
        assertTrue(set.remove(1));
        assertFalse(set.remove(1));
        assertFalse(set.add(2));
        assertTrue(set.remove(2));
        List<Integer> elements = set.scan();
        assertArrayEquals(new int[]{-1, 0}, elements.stream().mapToInt
                (x -> x).toArray());
    }

    @Test
    public void contains() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        set.add(0);
        assertTrue(set.contains(0));
        set.remove(0);
        assertFalse(set.contains(0));
    }

    @Test
    public void isEmpty() {
        LockFreeSet<Integer> set = new LockFreeSet<>();
        assertTrue(set.isEmpty());
        set.add(1);
        assertFalse(set.isEmpty());
        set.remove(1);
        assertTrue(set.isEmpty());
    }

    @Test
    public void iterator() {

    }
}