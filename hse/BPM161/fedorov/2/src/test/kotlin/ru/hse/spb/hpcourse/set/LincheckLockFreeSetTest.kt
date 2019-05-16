package ru.hse.spb.hpcourse.set

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.Test
import kotlin.random.Random

@StressCTest
class LincheckLockFreeSetTest {
    private val set = LockFreeSet<Int>()

    @Operation
    fun add(key: Int) = set.add(key)

    @Operation
    fun remove(key: Int) = set.remove(key)

    @Operation
    fun contains(key: Int) = set.contains(key)

    @Operation
    fun isEmpty() = set.isEmpty

    @Operation
    fun iterator(): List<Int> {
        val iterator = set.iterator()
        val values = mutableListOf<Int>()

        while (iterator.hasNext())
            values.add(iterator.next())

        return values
    }

    @Test
    fun test() {
        LinChecker.check(LincheckLockFreeSetTest::class.java)
    }

}