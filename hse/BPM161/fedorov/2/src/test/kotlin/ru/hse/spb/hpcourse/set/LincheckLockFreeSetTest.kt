package ru.hse.spb.hpcourse.set

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.Test

@Param(name = "key", gen = IntGen::class, conf = "1:5")
@StressCTest
class LincheckLockFreeSetTest {
    private val set = LockFreeSet<Int>()

    @Operation
    fun add(@Param(name = "key") key: Int) = set.add(key)

    @Operation
    fun remove(@Param(name = "key") key: Int) = set.remove(key)

    @Operation
    fun contains(@Param(name = "key") key: Int) = set.contains(key)

    @Operation
    fun isEmpty() = set.isEmpty

    @Operation
    fun iterator(): List<Int> = set.iterator().asSequence().toList()

    @Test
    fun test() {
        LinChecker.check(LincheckLockFreeSetTest::class.java)
    }

}