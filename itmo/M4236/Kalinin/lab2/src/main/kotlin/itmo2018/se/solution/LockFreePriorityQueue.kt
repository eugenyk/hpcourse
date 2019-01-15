package itmo2018.se.solution

import java.util.*
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicMarkableReference
import java.util.concurrent.atomic.AtomicReference

class LockFreePriorityQueue<T : Comparable<T>> : PriorityQueue<T>, AbstractQueue<T>() {
    private val fakeTail = Node<T>()
    private val fakeHead = Node(next = fakeTail)
    private var head = AtomicReference<Node<T>>(fakeHead)
    private val atomic_size = AtomicInteger(0)

    override fun offer(key: T): Boolean {
        while (true) {
            val foundNodes = find(key)
            val pred = foundNodes.first
            val curr = foundNodes.second
            val newNode = Node(key, curr)
            if (pred.nextAndMark.compareAndSet(curr, newNode, false, false)) {
                atomic_size.incrementAndGet()
                return true
            }

        }
    }

    override fun iterator(): MutableIterator<T> {
        throw UnsupportedOperationException()
    }

    override fun peek(): T? {
        while (true) {
            val first = fakeHead
            val curr = first.nextAndMark.reference

            if (curr == fakeTail) {
                return null
            } else {
                val cmk = curr.nextAndMark.isMarked
                val succ = curr.nextAndMark.reference
                if (cmk) {
                    first.nextAndMark.compareAndSet(curr, succ, false, false)
                } else {
                    return curr.key!!
                }
            }

        }
    }

    override fun poll(): T {
        while (true) {
            val first = fakeHead
            val curr = first.nextAndMark.reference

            if (curr == fakeTail) {
                throw NoSuchElementException()
            } else {
                val cmk = curr.nextAndMark.isMarked
                val succ = curr.nextAndMark.reference
                if (cmk) {
                    first.nextAndMark.compareAndSet(curr, succ, false, false)
                } else {
                    if (!curr.nextAndMark.compareAndSet(succ, succ, false, true)) {
                        continue
                    }
                    first.nextAndMark.compareAndSet(curr, succ, false, false)
                    atomic_size.decrementAndGet()
                    return curr.key!!
                }
            }

        }
    }

    override val size: Int
        get() = atomic_size.get()

    override fun isEmpty(): Boolean = peek() == null

    private class Node<T>(var key: T? = null, next: Node<T>? = null, marked: Boolean = false) {
        val nextAndMark = AtomicMarkableReference<Node<T>>(next, marked)
    }

    private fun find(key: T): Pair<Node<T>, Node<T>> {
        retry@ while (true) {
            var pred = head.get()
            var curr = pred.nextAndMark.reference
            var succ: Node<T>?
            while (true) {
                succ = curr.nextAndMark.reference
                val cmk = curr.nextAndMark.isMarked
                if (cmk) {
                    if (!pred.nextAndMark.compareAndSet(curr, succ, false, false)) {
                        continue@retry
                    }
                } else {
                    if (curr == fakeTail || curr.key!! >= key) {
                        return Pair(pred, curr)
                    }
                    pred = curr
                    curr = succ
                }
            }
        }
    }
}