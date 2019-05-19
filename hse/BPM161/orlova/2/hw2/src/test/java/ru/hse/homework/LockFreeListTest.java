package ru.hse.homework;

import org.junit.Before;
import org.junit.Test;
import ru.homework.LockFreeList;
import ru.homework.LockFreeSet;

import java.util.Iterator;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;

public class LockFreeListTest {

    private LockFreeSet<Integer> set;

    @Before
    public void setup() {
        set = new LockFreeList<>();
    }

    @Test
    public void testInitialEmpty() {
        assertTrue(set.isEmpty());
    }

    @Test
    public void testEmpty() {
        assertTrue(set.isEmpty());
        assertTrue(set.add(0));
        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertTrue(set.remove(0));
        assertTrue(set.isEmpty());
    }

    @Test
    public void testRemoveFromEmpty() {
        assertFalse(set.remove(0));
        assertFalse(set.remove(1));
    }

    @Test
    public void testAdd() {
        assertTrue(set.add(0));
        assertFalse(set.add(0));
        assertTrue(set.add(1));
        assertFalse(set.add(1));
    }

    @Test
    public void testContains() {
        assertFalse(set.contains(0));
        set.add(0);
        assertTrue(set.contains(0));
        set.remove(0);
        set.add(1);
        set.add(2);
        set.add(3);
        assertFalse(set.contains(0));
        assertTrue(set.contains(1));
    }

    @Test
    public void testRemove() {
        assertTrue(set.add(0));
        assertTrue(set.remove(0));
        assertFalse(set.remove(0));
        assertTrue(set.isEmpty());
        assertTrue(set.add(0));
    }

    @Test
    public void testIterator() {
        assertTrue(set.add(0));
        assertTrue(set.add(1));
        assertTrue(set.add(2));
        assertTrue(set.add(3));
        Iterator<Integer> iterator = set.iterator();
        assertEquals((Integer) 0, iterator.next());
        assertEquals((Integer) 1, iterator.next());
        assertEquals((Integer) 2, iterator.next());
        assertEquals((Integer) 3, iterator.next());
    }
}