package ru.spbhse.karvozavr.lockfreeset;

import org.junit.Test;

import static org.junit.Assert.*;

public class LockFreeSetTest {

    @Test
    public void testAddRemove() {
        LockFreeSet<Integer> set = new LockFreeSet<>();

        for (int i = 0; i < 10; ++i) {
            set.add(i);
        }

        for (int i = 0; i < 10; ++i) {
            assert set.contains(i);
        }

        for (int i = 0; i < 10; ++i) {
            set.remove(i);
        }

        for (int i = 0; i < 10; ++i) {
            assertFalse(set.contains(i));
        }
    }
}