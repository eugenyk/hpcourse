package org.ifmo

import com.devexperts.dxlab.lincheck.LinChecker
import com.devexperts.dxlab.lincheck.annotations.Operation
import com.devexperts.dxlab.lincheck.annotations.Param
import com.devexperts.dxlab.lincheck.paramgen.IntGen
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest
import junit.framework.TestCase.assertEquals
import org.junit.Test


@Param(name = "key", gen = IntGen::class, conf = "1:5")
@StressCTest
class PriorityQueueTest {
    private val queue = LockFreePriorityQueue<Int>()

    @Test
    fun `single thread test`() {
        assertEquals(true, queue.offer(2))
        assertEquals(2, queue.peek())
        assertEquals(true, queue.offer(3))
        assertEquals(2, queue.peek())
        assertEquals(true, queue.offer(1))
        assertEquals(1, queue.peek())

        assertEquals(1, queue.poll())
        assertEquals(2, queue.size)
        assertEquals(2, queue.peek())
        assertEquals(2, queue.poll())
        assertEquals(3, queue.peek())
        assertEquals(3, queue.poll())
        assertEquals(0, queue.size)

        queue.offer(1)
        queue.offer(5)
        queue.offer(3)
        queue.offer(4)
        queue.offer(2)
        queue.offer(0)
        assertEquals(queue.size, 6)

        assertEquals(queue.poll(), 0)
        assertEquals(queue.poll(), 1)
    }

    @Operation
    fun offer(@Param(name = "key") key: Int): Boolean {
        return queue.offer(key)
    }

    @Operation(handleExceptionsAsResult = [NullPointerException::class])
    fun poll(): Int {
        return queue.poll()!!
    }

    @Operation(handleExceptionsAsResult = [NullPointerException::class])
    fun peek(): Int {
        return queue.peek()!!
    }

    @Test
    fun lincheck() {
        LinChecker.check(PriorityQueueTest::class.java)
    }

}