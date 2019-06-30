package ru.hse.iisuslik

import org.junit.Test

import org.junit.Assert.*

class LockFreeSetTest {

    @Test
    fun isEmpty() {
        val set = LockFreeSet<Int>()
        assertTrue(set.isEmpty())
    }

    @Test
    fun isEmptyAfterRemove() {
        val set = LockFreeSet<Int>()
        assertTrue(set.isEmpty())
        set.add(1)
        assertFalse(set.isEmpty())
        set.remove(1)
        assertTrue(set.isEmpty())
    }

    @Test
    fun add1() {
        val set = LockFreeSet<Int>()
        set.add(1)
        assertTrue(set.contains(1))
    }

    @Test
    fun add2() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        assertTrue(set.contains(1))
        assertTrue(set.contains(2))
    }

    @Test
    fun addAgain() {
        val set = LockFreeSet<Int>()
        assertTrue(set.add(1))
        assertTrue(set.add(2))
        assertFalse(set.add(2))
        assertTrue(set.contains(1))
        assertTrue(set.contains(2))
    }

    @Test
    fun removeFirst() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(3)
        set.remove(1)
        assertFalse(set.contains(1))
        assertTrue(set.contains(2))
        assertTrue(set.contains(3))
    }

    @Test
    fun removeLast() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(3)
        set.remove(3)
        assertTrue(set.contains(1))
        assertTrue(set.contains(2))
        assertFalse(set.contains(3))
    }

    @Test
    fun removeMiddle() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(3)
        set.remove(2)
        assertTrue(set.contains(1))
        assertFalse(set.contains(2))
        assertTrue(set.contains(3))
    }


    @Test
    fun iterator() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(3)
        val it = set.iterator()
        val list = mutableListOf<Int>()
        it.forEach { list.add(it) }
        assertEquals(listOf(1, 2, 3), list)
    }

    @Test
    fun iteratorWith2Adds() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(2)
        set.add(3)
        val it = set.iterator()
        val list = mutableListOf<Int>()
        it.forEach { list.add(it) }
        assertEquals(listOf(1, 2, 3), list)
    }
}