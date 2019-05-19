package ru.spb.hse

import junit.framework.Assert.assertFalse
import junit.framework.Assert.assertTrue
import org.assertj.core.api.Assertions
import org.junit.Test

class BasicTests {
    @Test
    fun `check add`() {
        val set = LockFreeSet<Int>()
        assertTrue(set.add(1))
        assertTrue(set.add(2))
        assertFalse(set.add(1))
    }

    @Test
    fun `check contains`() {
        val set = LockFreeSet<Int>()
        assertFalse(set.contains(1))
        assertTrue(set.add(1))
        assertTrue(set.contains(1))
    }

    @Test
    fun `test remove`() {
        val set = LockFreeSet<Int>()
        assertFalse(set.remove(1))
        assertTrue(set.add(2))
        assertTrue(set.add(1))
        assertTrue(set.remove(1))
        assertFalse(set.remove(1))
    }

    @Test
    fun `test iterator`() {
        val set = LockFreeSet<Int>()
        set.add(1)
        set.add(2)
        set.add(3)
        set.add(4)
        set.remove(2)
        Assertions.assertThat(set.iterator().asSequence().toList())
            .containsExactly(1, 3, 4)
    }
}