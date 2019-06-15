package ru.hse.spb.hpcourse.set

import org.junit.Assert.*
import org.junit.Test

class AVLIteratorTest {
    @Test
    fun testIteratorEmpty() {
        assertEquals(false, AVLIterator(AVLNil<Int>()).hasNext())
    }

    @Test
    fun testHasNext() {
        assertEquals(true, AVLIterator(AVLNil<Int>().add(5)).hasNext())
    }

    @Test
    fun testNext() {
        val iterator = AVLIterator(AVLNil<Int>().add(5).add(4))
        assertEquals(true, iterator.hasNext())
        assertEquals(4, iterator.next())
        assertEquals(true, iterator.hasNext())
        assertEquals(5, iterator.next())
        assertEquals(false, iterator.hasNext())
    }

    @Test
    fun testLargeIterator() {
        val tree = AVLTreeTest.generateTree(1000)
        val iterator = AVLIterator(tree)

        for (i in 1..1000) {
            assertEquals(true, iterator.hasNext())
            assertEquals(i, iterator.next())
        }

        assertEquals(false, iterator.hasNext())
    }
}