package ru.hse.egorov

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.Test

@StressCTest
class LincheckLockFreeSetTest {
    private val set = LockFreeSet<Int>()

    @Operation
    fun add(key: Int) = set.add(key)

    @Operation
    fun remove( key: Int) = set.remove(key)

    @Operation
    fun contains(key: Int) = set.contains(key)

    @Operation
    fun isEmpty() = set.isEmpty

    @Operation
    fun iterator(): List<Int> = set.iterator().asSequence().toList()

    @Test
    fun test() {
        LinChecker.check(LincheckLockFreeSetTest::class.java)
    }

}