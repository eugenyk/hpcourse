package lockfree

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import org.junit.Test

@StressCTest(threads = 3, actorsBefore = 3, actorsAfter = 3)
class LinCheckTest {
    private val set = LockFreeSet<Int>()

    @Operation
    fun add(@Param(gen = IntGen::class) item: Int): Boolean {
        return set.add(item)
    }

    @Operation
    fun remove(@Param(gen = IntGen::class) item: Int): Boolean {
        return set.remove(item)
    }

    @Operation
    fun isEmpty(): Boolean {
        return set.isEmpty
    }

    @Operation
    fun contains(@Param(gen = IntGen::class) item: Int): Boolean {
        return set.contains(item)
    }

    @Operation
    fun iterator(): List<Int> {
        return set.iterator().asSequence().toList()
    }

    @Test
    fun test() {
        LinChecker.check(LinCheckTest::class.java)
    }
}