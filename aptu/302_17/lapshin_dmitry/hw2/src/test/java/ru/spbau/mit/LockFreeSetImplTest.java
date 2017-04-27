package ru.spbau.mit;

import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest {
    @Test
    public void simpleTest() {
        final int[] data = {10, -10, -9, 9, 8, -8, -7, 7, 6, -6, -5, 5, 4, -4, -3, 3, 2, -2, -1, 1, 0};
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();

        assertTrue(set.isEmpty());

        for (int i = 0; i != data.length + 1; i++) {
            for (int j = 0; j != i; j++) {
                assertTrue(set.contains(data[j]));
                assertFalse(set.add(data[j]));
            }
            if (i < data.length) {
                assertTrue(set.add(data[i]));
                assertFalse(set.isEmpty());
            }
            for (int j = i + 1; j < data.length; j++) {
                assertFalse(set.contains(data[j]));
                assertFalse(set.remove(data[j]));
            }
        }

        for (int i = 0; i != data.length + 1; i++) {
            for (int j = 0; j != i; j++) {
                assertFalse(set.contains(data[j]));
                assertFalse(set.remove(data[j]));
            }
            if (i < data.length) {
                assertFalse(set.isEmpty());
                assertTrue(set.remove(data[i]));
            }
            for (int j = i + 1; j < data.length; j++) {
                assertTrue(set.contains(data[j]));
                assertFalse(set.add(data[j]));
            }
        }

        assertTrue(set.isEmpty());
    }
}