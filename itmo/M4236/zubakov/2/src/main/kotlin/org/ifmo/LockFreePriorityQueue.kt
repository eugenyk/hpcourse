package org.ifmo

import java.util.*
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicMarkableReference

/**
 * inspired by https://timharris.uk/papers/2001-disc.pdf (really not inspired, but somehow taken from)
 */
class LockFreePriorityQueue<E : Comparable<E>> : PriorityQueue<E>, AbstractQueue<E>() {
    private val tail = Node<E>()
    private val head = Node(next = tail)
    private val totalSize = AtomicInteger(0)


    override fun iterator(): MutableIterator<E> {
        TODO("Method iterator should be overridden before using, but who cares")
    }

    override fun offer(e: E): Boolean {
        while (true) {
            val (prev, next) = findPosition(e)
            val newNext = Node(e, next)
            if (prev.changeNext(next, newNext)) {
                totalSize.incrementAndGet()
                return true
            }
        }
    }

    override fun peek(): E? {
        var first = head.next
        while (true) {
            if (first === head) {
                return null
            }

            if (!first.isMarked) {
                return first.value
            }

            first = first.next
        }
    }

    override fun poll(): E? {
        while (true) {
            val elem = findPosition(null).next
            if (elem === tail) {
                return null
            }
            val next = elem.next

            if (elem.mark(next)) {
                totalSize.decrementAndGet()
                return elem.value
            }
        }
    }

    override val size: Int
        get() = totalSize.get()

    private fun findPosition(value: E?): Position<E> {
        var prev = head
        var current = head.next
        var next: Node<E>
        while (true) {

            var tmpCurrent = head
            var tmpNext = head.next

            do {
                if (!tmpCurrent.isMarked) {
                    prev = tmpCurrent
                    current = tmpNext
                }
                tmpCurrent = tmpCurrent.next
                if (tmpCurrent === tail) {
                    break
                }
                tmpNext = tmpCurrent.next
            } while (tmpCurrent.isMarked || (value != null && tmpCurrent.value!! <= value))

            next = tmpCurrent

            if (current === next) {
                if (next !== tail && next.isMarked) {
                    continue
                } else {
                    return Position(prev, next)
                }
            }

            if (prev.changeNext(current, next)) {
                if (next === tail || !next.isMarked) {
                    return Position(prev, next)
                }
            }
        }
    }

    override fun isEmpty(): Boolean = size == 0

    private class Node<E>(val value: E? = null, next: Node<E>? = null) {
        private val mark = AtomicMarkableReference(next, false)
        val next: Node<E> get() = mark.reference!!
        val isMarked get() = mark.isMarked

        fun mark(next: Node<E>) =
                mark.compareAndSet(next, next, false, true)

        fun changeNext(expectedNext: Node<E>, newNext: Node<E>) =
                mark.compareAndSet(expectedNext, newNext, false, false)
    }

    private data class Position<E>(
            val prev: Node<E>,
            val next: Node<E>
    )
}