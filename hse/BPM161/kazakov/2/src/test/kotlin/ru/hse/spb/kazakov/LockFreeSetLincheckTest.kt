package ru.hse.spb.kazakov

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.jupiter.api.Test

@StressCTest
class LockFreeSetLincheckTest {
    private val set = LockFreeSet<Int>()

    @Operation
    fun add(value: Int) = set.add(value)

    @Operation
    fun contains(value: Int) = set.contains(value)

    @Operation
    fun isEmpty() = set.isEmpty()

    @Operation
    fun remove(value: Int) = set.remove(value)

    @Operation
    fun iterator(): List<Int> = set.iterator().asSequence().toList()

    @Test
    fun test() {
        LinChecker.check(LockFreeSetLincheckTest::class.java)
    }
}