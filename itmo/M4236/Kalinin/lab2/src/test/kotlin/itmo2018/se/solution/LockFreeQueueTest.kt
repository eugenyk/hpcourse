package itmo2018.se.solution

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import junit.framework.TestCase.assertEquals
import org.junit.Test
import java.util.NoSuchElementException


@Param(name = "key", gen = IntGen::class, conf = "1:5")
@StressCTest
class LockFreeQueueTest {
    private val q = LockFreePriorityQueue<Int>()

    @Operation
    fun offer(@Param(name = "key") key: Int): Boolean {
        return q.offer(key)
    }

    @Operation(handleExceptionsAsResult = arrayOf(NoSuchElementException::class))
    fun poll(): Int {
        return q.poll()
    }

    @Operation(handleExceptionsAsResult = arrayOf(NoSuchElementException::class))
    fun peek(): Int {
        return q.peek()
    }

    @Test
    fun runTest() {
        LinChecker.check(LockFreeQueueTest::class.java)
    }

    @Test
    fun singleThreadTest() {
        assertEquals(q.offer(1), true)
        assertEquals(q.peek(), 1)
        assertEquals(q.offer(3), true)
        assertEquals(q.peek(), 1)
        assertEquals(q.offer(0), true)
        assertEquals(q.peek(), 0)
        assertEquals(q.offer(2), true)
        assertEquals(q.offer(5), true)
        assertEquals(q.offer(3), true)
        assertEquals(q.size, 6)


        assertEquals(q.poll(), 0)
        assertEquals(q.size, 5)
        assertEquals(q.peek(), 1)
        assertEquals(q.poll(), 1)
        assertEquals(q.peek(), 2)
        assertEquals(q.poll(), 2)
        assertEquals(q.poll(), 3)
        assertEquals(q.poll(), 3)
        assertEquals(q.poll(), 5)
        assertEquals(q.size, 0)

        q.offer(1)
        q.offer(5)
        q.offer(3)
        q.offer(4)
        q.offer(2)
        q.offer(0)
        assertEquals(q.size, 6)

        assertEquals(q.poll(), 0)
        assertEquals(q.poll(), 1)
        assertEquals(q.add(0), true)
        q.add(0)
        assertEquals(q.poll(), 0)
        assertEquals(q.poll(), 0)
    }
}