package lockfree

import org.hamcrest.Matchers.containsInAnyOrder
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test

class LockFreeSetTest {

    private lateinit var set : LockFreeSet<Int>

    @Before
    fun setUp() {
        set = LockFreeSet()
    }

    @Test
    fun isEmpty() {
        assertTrue(set.isEmpty)
        fill(3)
        assertFalse(set.isEmpty)
        clear(3)
        assertTrue(set.isEmpty)
    }

    private fun clear(n: Int) {
        for (i in 1..n)
            set.remove(i)
    }

    @Test
    fun add() {
        assertTrue(set.add(1))
        assertFalse(set.add(1))
        assertTrue(set.add(2))
        assertFalse(set.add(2))
        assertTrue(set.add(3))
        assertFalse(set.add(1))
        assertFalse(set.add(3))
        assertFalse(set.add(2))
    }

    @Test
    fun remove() {
        repeat(2) {
            for (i in 1..3)
                assertFalse(set.remove(i))
            fill(3)
            for (i in 1..3)
                assertTrue(set.remove(i))
        }
    }

    private fun fill(n: Int) {
        for (i in 1..n)
            set.add(i)
    }

    @Test
    fun contains() {
        repeat(2) {
            for (i in 1..3)
                assertFalse(set.contains(i))
            fill(3)
            for (i in 1..3)
                assertTrue(set.contains(i))
            clear(3)
        }
    }

    @Test
    fun iterator() {
        assertFalse(set.iterator().hasNext())
        fill(3)
        assertThat(set.iterator().asSequence().toList(), containsInAnyOrder(1, 2, 3))
        clear(1)
        assertThat(set.iterator().asSequence().toList(), containsInAnyOrder(2, 3))
        clear(3)
        assertFalse(set.iterator().hasNext())
    }
}