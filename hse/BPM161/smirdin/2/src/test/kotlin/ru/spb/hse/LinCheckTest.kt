package ru.spb.hse

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.Test

@StressCTest(threads = 3, actorsBefore = 0, actorsPerThread = 5, actorsAfter = 0, invocationsPerIteration = 1000)
class LockFreeSetLinTest {
    val set = LockFreeSet<Int>()

    @Operation
    fun add(@Param(gen = IntGen::class) newItem: Int) = set.add(newItem)

    @Operation
    fun contains(@Param(gen = IntGen::class) item: Int) = set.contains(item)

    @Operation
    fun isEmpty() = set.isEmpty

    @Operation
    fun remove(@Param(gen = IntGen::class) item: Int) = set.remove(item)

    @Operation
    fun iterator(): List<Int> = set.iterator().asSequence().toList()

    @Test
    fun runner() {
        LinChecker.check(LockFreeSetLinTest::class.java)
    }
}