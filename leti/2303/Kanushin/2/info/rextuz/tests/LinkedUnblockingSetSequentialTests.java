package info.rextuz.tests;

import info.rextuz.hpcourse.LinkedUnblockingSet;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class LinkedUnblockingSetSequentialTests {

    @Test
    void test_add_sequential() {
        LinkedUnblockingSet<Integer> my_set = new LinkedUnblockingSet<>();

        assertTrue(my_set.add(42));
        assertFalse(my_set.add(42));
        assertTrue(my_set.add(35));
        assertFalse(my_set.add(35));
    }

    @Test
    void test_remove_sequential() {
        LinkedUnblockingSet<Integer> my_set = new LinkedUnblockingSet<>();

        assertFalse(my_set.remove(42));
        assertFalse(my_set.remove(35));

        my_set.add(42);
        assertTrue(my_set.remove(42));
        assertFalse(my_set.remove(35));

        my_set.add(35);
        assertFalse(my_set.remove(42));
        assertTrue(my_set.remove(35));
    }

    @Test
    void test_isEmpty_sequential() {
        LinkedUnblockingSet<Integer> my_set = new LinkedUnblockingSet<>();

        assertTrue(my_set.isEmpty());

        my_set.add(42);
        assertFalse(my_set.isEmpty());

        my_set.add(35);
        assertFalse(my_set.isEmpty());

        my_set.remove(35);
        assertFalse(my_set.isEmpty());

        my_set.remove(42);
        assertTrue(my_set.isEmpty());
    }

    @Test
    void test_contains_sequential() {
        LinkedUnblockingSet<Integer> my_set = new LinkedUnblockingSet<>();

        assertFalse(my_set.contains(42));
        assertFalse(my_set.contains(35));

        my_set.add(42);
        assertTrue(my_set.contains(42));
        assertFalse(my_set.contains(35));

        my_set.add(35);
        assertTrue(my_set.contains(42));
        assertTrue(my_set.contains(35));

        my_set.remove(35);
        assertTrue(my_set.contains(42));
        assertFalse(my_set.contains(35));

        my_set.remove(42);
        assertFalse(my_set.contains(42));
        assertFalse(my_set.contains(35));
    }
}

