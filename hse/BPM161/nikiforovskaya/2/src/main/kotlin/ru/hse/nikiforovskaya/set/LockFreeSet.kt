package ru.hse.nikiforovskaya.set

import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicMarkableReference


class LockFreeSet<T:Comparable<T>> : LockFreeSetInterface<T> {

    private val head : AtomicMarkableReference<ListNode<T>?> = AtomicMarkableReference(null, ListNode.UNMARKED)

    private var size : AtomicInteger = AtomicInteger(0)

    private class SearchResult<T : Comparable<T>>(val last: AtomicMarkableReference<ListNode<T>?>, val data: ListNode<T>?)

    override fun add(value: T): Boolean {
        val res = find(value)
        if (res.data != null && res.data.getKey() == value) {
            return false
        }

        var last = res.last

        while (!last.compareAndSet(null, ListNode(value), ListNode.UNMARKED, ListNode.UNMARKED)) {

            val res = find(value)
            if (res.data != null && res.data.getKey() == value) {
                return false
            }

            last = res.last
        }

        size.getAndIncrement()
        return true
    }

    override fun remove(value: T): Boolean {
        while (true) {
            val found = find(value)
            val current = found.data
            val last = found.last

            if (current != null) {
                val ref = current.next.reference
                if (current.next.compareAndSet(ref, ref, ListNode.UNMARKED, ListNode.MARKED)) {
                    actuallyRemove(last, current)
                    size.getAndDecrement()
                    return true
                }
            } else {
                return false
            }
        }
    }

    private fun actuallyRemove(last: AtomicMarkableReference<ListNode<T>?>, current: ListNode<T>) {
        var next : ListNode<T>? = current
        do {
            next = next!!.next.reference
        } while (next != null && next.isMarked())

        last.compareAndSet(current, next, ListNode.UNMARKED, ListNode.UNMARKED)
    }

    override fun contains(value: T): Boolean {
        val node = find(value).data
        return node != null
    }

    override fun isEmpty(): Boolean {
        return size.get() == 0
    }

    override fun iterator(): Iterator<T> {
        return getSnapshot().listIterator()
    }


    private fun find(value: T): SearchResult<T> {
        var last: AtomicMarkableReference<ListNode<T>?> = head
        var node: ListNode<T>? = head.reference

        while (node != null) {
            if (!node.isMarked() && value == node.getKey()) {
                return SearchResult(last, node)
            }
            last = node.next
            node = last.reference
        }
        return SearchResult(last, node)
    }

    private fun getSnapshot() : List<T> {
        while (true) {
            val first = getSomeList()
            val second = getSomeList()
            if (first == second) {
                return first
            }
        }
    }

    private fun getSomeList(): List<T> {
        val result = mutableListOf<T>()
        var node: ListNode<T>? = head.reference

        while (node != null) {
            if (!node.isMarked()) {
                result.add(node.getKey())
            }
            node = node.nextNode()
        }
        return result
    }
}