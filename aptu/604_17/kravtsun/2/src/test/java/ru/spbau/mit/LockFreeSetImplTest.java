package ru.spbau.mit;

import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest {
    private LockFreeSet<Integer> set;

    @Before
    public void setUp() {
        set = new LockFreeSetImpl<>();
    }

    @Test
    public void add() throws Exception {
        assertTrue(set.add(1));
        assertTrue(set.contains(1));

        assertTrue(set.add(2));
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));

        assertTrue(set.add(3));
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
        assertTrue(set.contains(3));
    }

    @Test
    public void remove() throws Exception {
        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertFalse(set.remove(1));

        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertFalse(set.remove(1));
    }

    @Test
    public void isEmpty() throws Exception {
        assertTrue(set.isEmpty());
        assertTrue(set.add(1));
        assertFalse(set.isEmpty());
        assertTrue(set.remove(1));
        assertTrue(set.isEmpty());
    }

    @Test
    public void limitsTest() throws Exception {
        testLimit(Integer.MIN_VALUE);
        testLimit(Integer.MAX_VALUE);
    }

    private void testLimit(Integer limitValue) {
        assertFalse(set.contains(limitValue));

        assertTrue(set.add(limitValue));
        assertFalse(set.add(limitValue));
        assertTrue(set.contains(limitValue));

        assertTrue(set.remove(limitValue));
        assertFalse(set.remove(limitValue));
        assertFalse(set.contains(limitValue));

        assertTrue(set.add(limitValue));
        assertFalse(set.add(limitValue));
        assertTrue(set.contains(limitValue));

        assertTrue(set.remove(limitValue));
        assertFalse(set.remove(limitValue));
        assertFalse(set.contains(limitValue));
    }

    @Test
    public void nullTest() {
        assertFalse(set.contains(null));

        assertTrue(set.add(null));
        assertFalse(set.add(null));
        assertTrue(set.contains(null));

        assertTrue(set.remove(null));
        assertFalse(set.remove(null));
        assertFalse(set.contains(null));
    }
}