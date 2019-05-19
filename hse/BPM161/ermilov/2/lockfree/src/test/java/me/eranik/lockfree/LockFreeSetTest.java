package me.eranik.lockfree;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;

import static org.junit.Assert.*;

public class LockFreeSetTest {
    @Test
    public void testContainsAfterAdd() {
        Set<Integer> set = new LockFreeSet<>();
        for (int i = 0; i < 10; ++i)
            assertTrue(set.add(i));
        for (int i = 0; i < 10; ++i)
            assertTrue(set.contains(i));
    }

    @Test
    public void testAdd() {
        Set<Integer> set = new LockFreeSet<>();
        for (int i = 0; i < 10; ++i)
            assertTrue(set.add(i));
        for (int i = 0; i < 10; ++i)
            assertFalse(set.add(i));
    }

    @Test
    public void testRemove() {
        Set<Integer> set = new LockFreeSet<>();
        for (int i = 0; i < 10; ++i)
            assertFalse(set.remove(i));
        for (int i = 0; i < 10; ++i)
            set.add(i);
        for (int i = 0; i < 10; ++i)
            assertTrue(set.remove(i));
        for (int i = 0; i < 10; ++i)
            assertFalse(set.remove(i));
    }

    @Test
    public void testContainsAfterRemoval() {
        Set<Integer> set = new LockFreeSet<>();
        for (int i = 0; i < 10; ++i)
            set.add(i);
        for (int i = 0; i < 5; ++i)
            set.remove(2 * i);
        for (int i = 0; i < 10; ++i) {
            if (i % 2 == 1) {
                assertTrue(set.contains(i));
            } else {
                assertFalse(set.contains(i));
            }
        }
    }

    @Test
    public void testEmpty() {
        Set<Integer> set = new LockFreeSet<>();
        assertTrue(set.isEmpty());
        for (int i = 0; i < 10; ++i)
            set.add(i);
        assertFalse(set.isEmpty());
        for (int i = 0; i < 5; ++i)
            set.remove(2 * i);
        assertFalse(set.isEmpty());
        for (int i = 0; i < 5; ++i)
            set.remove(2 * i + 1);
        assertTrue(set.isEmpty());
    }

    @Test
    public void testIterator() {
        ArrayList<Integer> input = new ArrayList<>();
        ArrayList<Integer> output = new ArrayList<>();
        for (int i = 0; i < 10; ++i)
            input.add(i);

        Set<Integer> set = new LockFreeSet<>();
        input.forEach(set::add);

        for (Iterator<Integer> iter = set.iterator(); iter.hasNext();) {
            Integer value = iter.next();
            output.add(value);
        }

        output.sort(Integer::compareTo);

        assertArrayEquals(input.toArray(), output.toArray());
    }
}
