package ru.itmo.hpc

import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import org.junit.Test
import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest

@StressCTest
@Param(name = "e", gen = IntGen::class, conf = "1:10000")
class PriorityQueueLinearizationTest {

    val queue = LockFreePriorityQueue<Int>()

    @Operation
    fun offer(@Param(name = "e") e: Int) = queue.offer(e)

    @Operation
    fun peek(): Int? = queue.peek()

    @Operation
    fun poll(): Int? = queue.poll()

    @Operation
    fun empty(): Boolean = queue.isEmpty()

    @Test
    fun runTest() {
        LinChecker.check(PriorityQueueLinearizationTest::class.java)
    }
}