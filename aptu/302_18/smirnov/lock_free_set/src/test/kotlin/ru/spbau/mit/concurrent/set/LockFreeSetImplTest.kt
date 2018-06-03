package ru.spbau.mit.concurrent.set

import org.junit.Test
import java.util.concurrent.ConcurrentLinkedQueue
import kotlin.concurrent.thread

private fun <T: Comparable<T>> LockFreeSetImpl<T>.addAll(collection: Collection<T>) =
        collection.forEach { element -> add(element) }

class LockFreeSetImplTest {
    private companion object {
        const val DATA_SIZE = 1000
        const val THREAD_COUNT = 10
        const val TEST_RUNS = 20
    }

    @Test
    fun addTest() {
        val data = List(DATA_SIZE) { n -> n }
        val concurrentLinkedQueue = ConcurrentLinkedQueue<Int>()

        for (i in 1..TEST_RUNS) {
            concurrentLinkedQueue.addAll(data)
            val lockFreeSet = LockFreeSetImpl<Int>()

            val threads = List(THREAD_COUNT) { _ ->
                thread {
                    while (true) {
                        val element = concurrentLinkedQueue.poll()
                        assert(lockFreeSet.add(element ?: return@thread))
                    }
                }
            }

            threads.forEach(Thread::join)
            for (element in data) {
                assert(lockFreeSet.contains(element)) { "$element missing" }
            }
        }
    }


    @Test
    fun removeTest() {
        val data = List(DATA_SIZE) { n -> n }
        val concurrentLinkedQueue = ConcurrentLinkedQueue<Int>()
        val shouldRemove = { element: Int -> element % 2 == 0 }

        for (i in 1..TEST_RUNS) {
            concurrentLinkedQueue.addAll(data)
            val lockFreeSet = LockFreeSetImpl<Int>()
            lockFreeSet.addAll(data)

            val threads = List(THREAD_COUNT) { _ ->
                thread {
                    while (true) {
                        val element = concurrentLinkedQueue.poll()
                        if (element == null) {
                            return@thread
                        } else if (shouldRemove(element)) {
                            assert(lockFreeSet.remove(element))
                        }
                    }
                }
            }

            threads.forEach(Thread::join)
            for (element in data) {
                if (shouldRemove(element)) {
                    assert(!lockFreeSet.contains(element)) { "$element should be missing" }
                } else {
                    assert(lockFreeSet.contains(element)) { "$element should not be missing" }
                }
            }
        }
    }

    @Test
    fun simpleTest() {
        for (i in 1..TEST_RUNS) {
            val lockFreeSet = LockFreeSetImpl<Int>()

            val threads = List(THREAD_COUNT) { id ->
                thread {
                    val threadData = List(DATA_SIZE) { n -> id * DATA_SIZE + n }

                    for (element in threadData) {
                        assert(lockFreeSet.add(element))
                    }
                    for (element in threadData) {
                        assert(lockFreeSet.contains(element))
                    }
                    for (element in threadData) {
                        assert(lockFreeSet.remove(element))
                    }
                }
            }

            threads.forEach(Thread::join)
            assert(lockFreeSet.isEmpty()) { "set should be empty" }
        }
    }
}