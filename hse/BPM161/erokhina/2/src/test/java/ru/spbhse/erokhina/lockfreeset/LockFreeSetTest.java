package ru.spbhse.erokhina.lockfreeset;

import org.apache.commons.collections4.IteratorUtils;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.*;
import static org.junit.Assert.assertFalse;

public class LockFreeSetTest {
    @Test
    public void simpleAddTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertTrue(set.add(1));
        assertTrue(set.contains(1));
        assertFalse(set.add(1));
        assertTrue(set.contains(1));
    }

    @Test
    public void twoElementsAddTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertTrue(set.add(1));
        assertTrue(set.contains(1));
        assertTrue(set.add(2));
        assertTrue(set.contains(2));

        List<Integer> expected = Arrays.asList(1, 2);
        List<Integer> actual = IteratorUtils.toList(set.iterator());

        assertEquals(actual, expected);
    }

    @Test
    public void complexAddTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        for (int i = 1; i <= 30; i++) {
            assertTrue(set.add(i));
        }

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.add(i));
        }

        for (int i = 1; i <= 30; i++) {
            assertTrue(set.contains(i));
        }
    }

    @Test
    public void simpleRemoveTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertFalse(set.remove(1));
        set.add(1);
        assertTrue(set.contains(1));
        assertTrue(set.remove(1));
        assertFalse(set.contains(1));
    }

    @Test
    public void twoElementsRemoveTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertFalse(set.remove(1));
        assertFalse(set.remove(2));
        set.add(1);
        set.add(2);
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
        assertTrue(set.remove(1));
        assertTrue(set.remove(2));
        assertFalse(set.contains(1));
        assertFalse(set.contains(2));
    }

    @Test
    public void complexRemoveTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        for (int i = 1; i <= 30; i++) {
            set.add(i);
        }

        for (int i = 1; i <= 30; i++) {
            assertTrue(set.remove(i));
        }

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.remove(i));
        }

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.contains(i));
        }
    }

    @Test
    public void simpleContainsTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertFalse(set.contains(1));
        set.add(1);
        assertTrue(set.contains(1));
        set.remove(1);
        assertFalse(set.contains(1));
    }

    @Test
    public void complexContainsTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.contains(i));
        }

        for (int i = 1; i <= 30; i++) {
            set.add(i);
        }

        for (int i = 1; i <= 30; i++) {
            assertTrue(set.contains(i));
        }

        for (int i = 1; i <= 30; i++) {
            set.remove(i);
        }

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.contains(i));
        }
    }

    @Test
    public void simpleIsEmptyTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertTrue(set.isEmpty());
        set.add(1);
        assertFalse(set.isEmpty());
        set.remove(1);
        assertTrue(set.isEmpty());
    }

    @Test
    public void complexIsEmptyTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        assertTrue(set.isEmpty());

        for (int i = 1; i <= 30; i++) {
            set.add(i);
            assertFalse(set.isEmpty());
        }

        for (int i = 1; i <= 30; i++) {
            assertFalse(set.isEmpty());
            set.remove(i);
        }

        assertTrue(set.isEmpty());
    }

    @Test
    public void emptySetIteratorTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        List<Integer> expected = Collections.emptyList();
        List<Integer> actual = IteratorUtils.toList(set.iterator());

        assertEquals(actual, expected);
    }

    @Test
    public void complexIteratorTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        for (int i = 30; i >= 1; i--) {
            set.add(i);
        }

        List<Integer> expected = new ArrayList<>();

        for (int i = 1; i <= 30; i++) {
            expected.add(i);
        }

        List<Integer> actual = IteratorUtils.toList(set.iterator());

        assertEquals(actual, expected);
    }
}
