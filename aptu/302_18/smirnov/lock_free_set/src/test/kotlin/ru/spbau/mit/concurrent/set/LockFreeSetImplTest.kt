package ru.spbau.mit.concurrent.set

import org.junit.Test
import java.util.concurrent.ConcurrentLinkedQueue
import kotlin.concurrent.thread

class LockFreeSetImplTest {
    private companion object {
        const val ARRAY_LENGTH = 1000
        const val THREAD_COUNT = 10
        const val TEST_RUNS = 20
    }

    @Test
    fun addTest() {
        val data = List(ARRAY_LENGTH) { n -> n }
        val concurrentLinkedQueue = ConcurrentLinkedQueue<Int>()

        for (i in 1..TEST_RUNS) {
            concurrentLinkedQueue.addAll(data)
            val lockFreeSet = LockFreeSetImpl<Int>()

            val threads = List(THREAD_COUNT) { _ ->
                thread {
                    while (true) {
                        val element = concurrentLinkedQueue.poll()
                        lockFreeSet.add(element ?: return@thread)
                    }
                }
            }

            threads.forEach(Thread::join)
            for (element in data) {
                assert(lockFreeSet.contains(element), { "$element missing" })
            }
        }
    }
}