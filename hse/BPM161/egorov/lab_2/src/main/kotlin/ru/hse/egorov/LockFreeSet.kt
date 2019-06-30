package ru.hse.egorov

import java.util.concurrent.atomic.AtomicMarkableReference

class LockFreeSet<T : Comparable<T>> : LockFreeSetInterface<T> {
    private val head = AtomicMarkableReference(
        Node(
            null,
            AtomicMarkableReference<Node<T>>(null, false)
        ), true
    )


    override val isEmpty: Boolean
        get() {
            var firstSnapshot = takeSnapshot()
            var secondSnapshot = takeSnapshot()
            while (firstSnapshot != secondSnapshot) {
                firstSnapshot = takeSnapshot()
                secondSnapshot = takeSnapshot()
            }
            return firstSnapshot.isEmpty()
        }

    override fun add(value: T): Boolean {
        val node = Node(value, AtomicMarkableReference<Node<T>>(null, false))

        while (true) {
            val (prev, current) = find(value)

            if (current != null) {
                return false
            } else {
                if (prev.next.compareAndSet(null, node, false, false)) {
                    return true
                }
            }
        }
    }

    override fun remove(value: T): Boolean {
        while (true) {
            val (prev, current) = find(value)

            if (current == null)
                return false

            val ref = current.next.reference
            if (current.next.compareAndSet(ref, ref, false, true)) {
                physicallyRemove(prev, current)
                return true
            }
        }
    }


    override fun contains(value: T): Boolean {
        var cur = head.reference.next.reference
        while (cur != null) {
            if (cur.value == value)
                return !isRemoved(cur)
            cur = cur.next.reference
        }
        return false
    }

    override fun iterator(): Iterator<T> {
        var firstSnapshot = takeSnapshot()
        var secondSnapshot = takeSnapshot()
        while (firstSnapshot != secondSnapshot) {
            firstSnapshot = takeSnapshot()
            secondSnapshot = takeSnapshot()
        }
        return firstSnapshot.map { ref -> ref.value!! }.iterator()
    }

    private fun takeSnapshot(): List<Node<T>> {
        var cur = head.reference.next.reference
        val set = mutableListOf<Node<T>>()

        while (cur != null) {
            set.add(cur)
            cur = cur.next.reference
        }

        return set.filter { ref -> !isRemoved(ref) }
    }

    private fun find(value: T): Pair<Node<T>, Node<T>?> {
        var prev: Node<T>
        var node: Node<T>?
        prev = head.reference
        node = prev.next.reference
        while (node != null) {
            if (!isRemoved(node) && value == node.value) {
                return Pair(prev, node)
            }
            prev = node
            node = node.next.reference
        }

        return Pair<Node<T>, Node<T>?>(prev, node)
    }

    private fun isRemoved(node: Node<T>): Boolean {
        return node.next.isMarked
    }

    private fun physicallyRemove(prev: Node<T>, node: Node<T>) {
        var next: Node<T>? = node
        do {
            next = next!!.next.reference
        } while (next != null && isRemoved(next))

        prev.next.compareAndSet(node, next, false, false)
    }

    companion object {
        private data class Node<T>(val value: T?, val next: AtomicMarkableReference<Node<T>>)
    }
}