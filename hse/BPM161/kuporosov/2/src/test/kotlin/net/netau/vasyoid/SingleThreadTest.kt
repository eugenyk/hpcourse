package net.netau.vasyoid

import org.junit.Assert.*
import org.junit.Test
import java.util.*
import java.util.concurrent.ConcurrentHashMap
import kotlin.random.Random

class SingleThreadTest {

    @Test
    fun stress() {
        val exp = Collections.newSetFromMap(ConcurrentHashMap<Int, Boolean>())
        val act = LockFreeSet<Int>()
        for (i in 0..10000) {
            val value = Random.nextInt(10)
            val action = Random.nextInt(4)
            val (expected, actual) = when (action) {
                0 -> exp.add(value) to act.add(value)
                1 -> exp.remove(value) to act.remove(value)
                2 -> exp.contains(value) to act.contains(value)
                else -> exp.isEmpty() to act.isEmpty()
            }
            assertEquals(expected, actual)
            val expectedElems = exp.iterator().asSequence().toList().sorted()
            val actualElems = act.iterator().asSequence().toList().sorted()
            assertEquals(expectedElems, actualElems)
        }
    }
}