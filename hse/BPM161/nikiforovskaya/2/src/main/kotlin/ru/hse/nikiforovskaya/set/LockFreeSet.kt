package ru.hse.nikiforovskaya.set

import java.util.concurrent.atomic.AtomicMarkableReference


class LockFreeSet<T:Comparable<T>> : LockFreeSetInterface<T> {

    private val head : AtomicMarkableReference<ListNode<T>?> = AtomicMarkableReference(null, ListNode.UNMARKED)

    private class SearchResult<T : Comparable<T>>(val last: AtomicMarkableReference<ListNode<T>?>, val data: ListNode<T>?)

    override fun add(value: T): Boolean {
        val res = find(value)
        if (res.data != null && res.data.getKey() == value && !res.data.isMarked()) {
            return false
        }

        var last = res.last

        while (!last.compareAndSet(null, ListNode(value), ListNode.UNMARKED, ListNode.UNMARKED)) {

            if (last.isMarked) {
                tryAllRemoves()
            }

            val res = find(value)
            if (res.data != null && res.data.getKey() == value && !res.data.isMarked()) {
                return false
            }

            last = res.last
        }
        return true
    }

    private fun tryAllRemoves() {
        var last = head
        var node: ListNode<T>? = head.reference

        while (node != null) {
            if (node.isMarked()) {
                actuallyRemove(last, node)
            }
            last = node.next
            node = node.nextNode()
        }
    }

    override fun remove(value: T): Boolean {
        while (true) {
            val found = find(value)
            val current = found.data
            val last = found.last

            if (current != null) {
                val ref = current.nextNode()
                if (current.next.compareAndSet(ref, ref, ListNode.UNMARKED, ListNode.MARKED)) {
                    actuallyRemove(last, current)
                    return true
                }
            } else {
                return false
            }
        }
    }

    private fun actuallyRemove(last: AtomicMarkableReference<ListNode<T>?>, current: ListNode<T>) {
        var next : ListNode<T>? = current
        while (next != null && next.isMarked()) {
            next = next.nextNode()
        }
        last.compareAndSet(current, next, ListNode.UNMARKED, ListNode.UNMARKED)
    }

    override fun contains(value: T): Boolean {
        val node = find(value).data
        return node != null
    }

    override fun isEmpty(): Boolean {
        return !iterator().hasNext()
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
                return first.first
            }
        }
    }

    private fun getSomeList(): Pair<List<T>, AtomicMarkableReference<ListNode<T>?>> {
        val result = mutableListOf<T>()
        var last = head
        var node: ListNode<T>? = head.reference

        while (node != null) {
            if (!node.isMarked()) {
                result.add(node.getKey())
            }
            last = node.next
            node = node.nextNode()
        }
        return Pair(result, last)
    }
}