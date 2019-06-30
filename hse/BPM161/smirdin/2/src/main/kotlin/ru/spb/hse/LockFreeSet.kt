package ru.spb.hse

import java.util.concurrent.atomic.AtomicMarkableReference

class LockFreeSet<T> : ILockFreeSet<T>
        where T : Comparable<T> {

    private val head: Node<T> = Node(null, AtomicMarkableReference<Node<T>?>(null, true))

    override val isEmpty: Boolean
        get() = !iterator().hasNext()

    override fun add(value: T): Boolean {
        while (true) {
            val (prev, cur) = findPrevAndCur(value)
            if (cur != null && cur.value == value) return false

            val newNode = Node<T>(value, AtomicMarkableReference(null, true))
            if (prev.nxt.compareAndSet(null, newNode, true, true))
                return true
        }
    }

    override fun remove(value: T): Boolean {
        while (true) {
            val (_, cur) = findPrevAndCur(value)
            if (cur == null) return false

            val toMarkDeleted = cur.nxt
            if (toMarkDeleted.compareAndSet(toMarkDeleted.reference, toMarkDeleted.reference, true, false)) {
                return true
            }
            continue
        }
    }

    override fun contains(value: T): Boolean {
        return findPrevAndCur(value).second?.value == value
    }

    override fun iterator(): Iterator<T> {
        while (true) {
            val snapshot1 = getSnapshot()
            val snapshot2 = getSnapshot()
            if (snapshot1.size == snapshot2.size) {
                for ((el1, el2) in snapshot1.zip(snapshot2)) {
                    if (el1 !== el2) continue
                }
            }

            return snapshot1.map { it -> it.value!! }.iterator()
        }
    }

    private fun getSnapshot(): List<Node<T>> {
        var cur = head.nxt.reference
        val nodes = mutableListOf<Node<T>>()
        while (cur != null) {
            if (cur.nxt.isMarked)
                nodes.add(cur)
            cur = cur.nxt.reference
        }
        return nodes
    }

    private fun findPrevAndCur(value: T): Pair<Node<T>, Node<T>?> {
        while (true) {
            var cur = head
            var nxt = head.nxt.reference

            loop@ while (nxt != null) {
                // Physically removing nodes
                if (!nxt.nxt.isMarked) {
                    cur.nxt.compareAndSet(nxt, nxt.nxt.reference, true, true)
                    nxt = cur.nxt.reference
                    if (nxt == null)
                        break@loop
                }

                if (nxt.value == value && nxt.nxt.isMarked)
                    return Pair(cur, nxt)
                cur = nxt
                nxt = cur.nxt.reference
            }
            return Pair(cur, nxt)
        }
    }

    data class Node<T>(val value: T?, var nxt: AtomicMarkableReference<Node<T>?>)
}