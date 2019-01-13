package ru.itmo.hpc

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.util.concurrent.ThreadLocalRandom
import java.util.concurrent.atomic.AtomicInteger
import kotlin.concurrent.thread

class LockFreePriorityQueueTest {

    private val queue = LockFreePriorityQueue<Int>();

    @Test
    fun `test offer works correctly in parallel`() {
        val list = permutation(10000)
        list.parallelStream().forEach { queue.offer(it) }

        for (i in 1..10000) {
            assertEquals(i, queue.peek())
            assertEquals(i, queue.poll())
        }
    }

    @Test
    fun `test offer and poll work correctly in parallel, consequently`() {
        val list = permutation(10000)

        // Add 10000 shuffled numbers to queue
        val offeringThreads = mutableListOf<Thread>()
        for (i in 0 until 10) {
            offeringThreads.add(
                    thread(start = true) {
                        list.subList(1000 * i, 1000 * (i + 1)).forEach { queue.offer(it) }
                    }
            )
        }
        offeringThreads.forEach { it.join() }

        // Remove first 5000 numbers from queue
        val pollingThreads = mutableListOf<Thread>()
        for (i in 0 until 5) {
            pollingThreads.add(
                    thread(start = true) {
                        for (j in 0 until 1000) {
                            queue.poll()
                        }
                    }
            )
        }

        pollingThreads.forEach { it.join() }

        for (i in 5001..10000) {
            assertEquals(i, queue.peek())
            assertEquals(i, queue.poll())
        }
    }

    @Test
    fun `test offer and poll work correctly in parallel, together`() {
        val list = permutation(10000)

        val threads = mutableListOf<Thread>()
        for (i in 0 until 10) {
            threads.add(
                    thread(start = true) {
                        list.subList(1000 * i, 1000 * (i + 1)).forEach {
                            queue.offer(it)
                            queue.poll()
                        }
                    }
            )
        }
        threads.forEach { it.join() }

        assertTrue(queue.isEmpty())
    }

    @Test
    fun `test offer and poll work correctly in parallel, mixing`() {
        val list = permutation(10000)

        // Add 10000 shuffled numbers to queue
        val offeringThreads = mutableListOf<Thread>()
        for (i in 0 until 10) {
            offeringThreads.add(
                    thread(start = true) {
                        list.subList(1000 * i, 1000 * (i + 1)).forEach { queue.offer(it) }
                    }
            )
        }

        // Remove first 5000 numbers from queue
        val pollingThreads = mutableListOf<Thread>()

        // We can poll when queue is empty, and therefore remove nothing.
        // So, we must consider these situations when check total size after threads work.
        val pollsOnEmptyQueue = AtomicInteger(0)

        for (i in 0 until 5) {
            pollingThreads.add(
                    thread(start = true) {
                        for (j in 0 until 1000) {
                            if (queue.poll() == null) {
                                pollsOnEmptyQueue.incrementAndGet()
                            }
                        }
                    }
            )
        }

        offeringThreads.forEach { it.join() }
        pollingThreads.forEach { it.join() }

        // We cannot say anything about remaining values, because we don't know
        // at which moment every value was offered.
        // For example, if 1 was the last offered value, it will be at the head of queue.
        // But anyway, we can check the total order of numbers.
        var lastPolledValue = queue.poll()!!
        for (i in (5001 - pollsOnEmptyQueue.get() + 1)..10000) {
            assertTrue(lastPolledValue < queue.peek()!!)
            lastPolledValue = queue.poll()!!
        }
    }

    companion object {
        fun <T> shuffle(elements: MutableList<T>) {
            val random = ThreadLocalRandom.current()
            for (i in elements.size - 1 downTo 1) {
                val j = random.nextInt(i + 1)
                val tmp = elements[j]
                elements[j] = elements[i]
                elements[i] = tmp
            }
        }

        // Random permutation of first N natural numbers.
        // Applicable for testing `offer` and `poll`.
        fun permutation(limit: Int): List<Int> {
            val sequentialList = generateSequence(1) { it + 1 }.take(limit).toMutableList()
            shuffle(sequentialList)
            return sequentialList
        }
    }
}