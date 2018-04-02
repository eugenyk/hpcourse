package ru.spbau.mit.concurrent.set

import java.util.concurrent.atomic.AtomicMarkableReference

private typealias NodeRef<T> = AtomicMarkableReference<Node<T>>

private class Node<T>(val value: T, val next: NodeRef<T> = NodeRef(null, false))

private fun <T> propagateForward(markedNext: NodeRef<T>, next: Node<T>) =
        markedNext.compareAndSet(next, next.next.reference, false, false)

class LockFreeSetImpl<in T : Comparable<T>>: LockFreeSet<T> {
    private val head = NodeRef<T>(null, false)

    private fun search(value: T): Pair<Node<T>?,NodeRef<T>> {
        fun helper(prev: Node<T>?, cur: NodeRef<T>): Pair<Node<T>?,NodeRef<T>> =
                when {
                    cur.isMarked  -> {
                        propagateForward(
                                prev?.next ?: cur,
                                cur.reference
                        )               // if prev is null then cur is head
                        search(value)   // start over
                    }
                    cur.reference == null || value <= cur.reference.value ->
                        Pair(prev, cur) // we have either reached the end or found the expected position
                    else                              ->
                        helper(cur.reference, cur.reference.next) // simply take next node
                }

        return helper(null, head)
    }

    override fun add(value: T): Boolean {
        val (prev, cur) = search(value)
        return if (value == cur.reference.value) {
            false          // already in set
        } else {
            val next = Node(value, cur)
            if ((prev?.next ?: head).compareAndSet(cur.reference, next, false, false)) {
                true       // add succeeded
            } else {
                add(value) // retry otherwise
            }
        }
    }

    override fun remove(value: T): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun contains(value: T): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun isEmpty(): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

}