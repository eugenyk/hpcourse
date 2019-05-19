package ru.hse.egorov

import java.util.concurrent.atomic.AtomicMarkableReference

class LockFreeSet<T : Comparable<T>> : LockFreeSetInterface<T> {
    private val tail = AtomicMarkableReference(Node<T?>(null, null), true)
    private val head = AtomicMarkableReference(Node(null, tail), true)


    override val isEmpty: Boolean
        get() = head.reference.next == tail

    override fun add(value: T): Boolean {
        while (true) {
            val (prev, node) = find(value)
            if (node != tail && node!!.reference.value == value) {
                return false
            }
            if (prev.reference.next!!.compareAndSet(node.reference, Node(value, node), false, false)) {
                return true
            }
        }
    }

    override fun remove(value: T): Boolean {
        while (true) {
            val (prev, node) = find(value)
            if (node!!.reference.value != value || node.isMarked) {
                return false
            }
            val next = node.reference.next
            if (node.attemptMark(node.reference, true)) {
                prev.reference.next!!.compareAndSet(node.reference, next!!.reference, false, false)
                return true
            }
        }
    }

    override fun contains(value: T): Boolean {
        var cur = head.reference.next
        while (cur != tail && cur!!.reference.value!! <= value) {
            if (cur.reference.value == value)
                return !cur.isMarked
            cur = cur.reference.next
        }
        return false
    }

    override fun iterator(): Iterator<T> {
        var firstSnapshot = takeSnapshot()
        var secondSnapshot = takeSnapshot()
        while(firstSnapshot != secondSnapshot){
             firstSnapshot = takeSnapshot()
             secondSnapshot = takeSnapshot()
        }
        return firstSnapshot.iterator()
    }

    private fun takeSnapshot(): List<T> {
        var cur = head.reference.next
        val set = mutableListOf<AtomicMarkableReference<Node<T?>>>()

        while (cur != tail) {
                set.add(cur!!)
            cur = cur.reference.next
        }

        return set.filter{ ref -> !ref.isMarked}.map{ref -> ref.reference.value!!}
    }

    private fun find(value: T): Pair<AtomicMarkableReference<Node<T?>>, AtomicMarkableReference<Node<T?>>?> {
        while (true) {
            var cur = head.reference.next
            var prev = head
            var node = head.reference.next
            while (cur != tail && cur!!.reference.value!! < value) {
                if (!cur.isMarked) {
                    prev = cur
                    node = prev.reference.next
                }
                cur = cur.reference.next as AtomicMarkableReference<Node<T?>>
            }

            if (prev.reference.next!!.compareAndSet(node!!.reference, cur.reference, false, false)) {
                if (cur.isMarked) {
                    continue
                }
                return Pair(prev, node)
            }
        }
    }

    companion object {
        private data class Node<T>(val value: T?, val next: AtomicMarkableReference<Node<T>>?)
    }
}