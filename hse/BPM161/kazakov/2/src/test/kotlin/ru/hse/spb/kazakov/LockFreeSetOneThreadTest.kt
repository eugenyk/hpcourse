package ru.hse.spb.kazakov

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test

class LockFreeSetOneThreadTest {
    @Test
    fun testInitialEmpty() {
        assertTrue(LockFreeSet<Char>().isEmpty())
    }

    @Test
    fun testContainsAddedElement() {
        val set = LockFreeSet<Int>()
        set.add(1)
        assertTrue(set.contains(1))
    }

    @Test
    fun testDoesNotContainsNotExistingElement() {
        assertFalse(LockFreeSet<Int>().contains(1))
    }

    @Test
    fun testDoesNotContainsRemovedElement() {
        val set = LockFreeSet<Int>()
        set.add(-11)
        set.remove(-11)
        assertFalse(set.contains(-11))
    }

    @Test
    fun testAddNewElement() {
        assertTrue(LockFreeSet<Int>().add(421))
    }

    @Test
    fun testAddExistingElement() {
        val set = LockFreeSet<Int>()
        set.add(54)
        assertFalse(set.add(54))
    }

    @Test
    fun testAddRemovedElement() {
        val set = LockFreeSet<Int>()
        set.add(76)
        set.remove(76)
        assertTrue(set.add(76))
    }

    @Test
    fun testRemovedNotExistingElement() {
        val set = LockFreeSet<Int>()
        set.add(76)
        assertFalse(set.remove(0))
    }

    @Test
    fun testRemoveAddedElement() {
        val set = LockFreeSet<Int>()
        set.add(-1)
        assertTrue(set.remove(-1))
    }

    @Test
    fun testAddRemoveMultipleElements() {
        val set = LockFreeSet<Int>()

        assertTrue(set.add(54))
        assertTrue(set.add(-43))
        assertTrue(set.remove(-43))
        assertTrue(set.add(6543))
        assertTrue(set.add(-43))
        assertTrue(set.remove(54))

        assertTrue(set.contains(-43))
        assertTrue(set.contains(6543))
        assertFalse(set.contains(-54))
    }

    @Test
    fun testIterator() {
        val set = LockFreeSet<Int>()
        set.add(7)
        set.add(2)
        set.add(-1)
        set.add(324)

        val expectedList = listOf(-1, 2, 7, 324)
        assertIterableEquals(expectedList, set)
    }
}