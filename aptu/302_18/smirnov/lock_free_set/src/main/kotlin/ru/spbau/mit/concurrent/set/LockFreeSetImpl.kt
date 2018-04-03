package ru.spbau.mit.concurrent.set

import java.util.concurrent.atomic.AtomicMarkableReference

private typealias NodeRef<T> = AtomicMarkableReference<Node<T>>

private sealed class Node<T>(open val value: T?, next: Node<T>?) {
    private val nextRef = NodeRef(next, false)

    val next: Node<T>?
        get() = nextRef.reference

    val isMarkedForDeletion
        get() = nextRef.isMarked

    fun tryCASNext(
            expectedNext: Node<T>?,
            newNext: Node<T>?,
            expectedMark: Boolean,
            newMark: Boolean
    ) = nextRef.compareAndSet(expectedNext, newNext, expectedMark, newMark)

    fun tryMarkForDeletion() = nextRef.attemptMark(nextRef.reference, true)
}

private class HeadNode<T>(next: Node<T>? = null): Node<T>(null, next)

private class TailNode<T>(override val value: T, next: Node<T>?): Node<T>(null, next)

private fun <T> propagateForward(prev: Node<T>, markedCur: Node<T>) =
        prev.tryCASNext(markedCur, markedCur.next, false, false)

class LockFreeSetImpl<T : Comparable<T>> : LockFreeSet<T> {
    private val head = HeadNode<T>()

    private tailrec fun search(value: T): Pair<Node<T>,Node<T>?> {
        tailrec fun helper(prev: Node<T>, cur: Node<T>?): Pair<Node<T>,Node<T>?>? =
                when {
                    (cur != null && cur.isMarkedForDeletion) -> {
                        propagateForward(
                                prev,
                                cur
                        )
                        // remove marked
                        // and indicate outer function to start over
                        null
                    }
                    (cur == null || (cur is TailNode && value <= cur.value)) ->
                        // we have found the desired position
                        Pair(prev, cur)
                    else ->
                        // otherwise simply go to the next node
                        helper(cur, cur.next)
                }

        return helper(head, head.next) ?: search(value)
    }

    override tailrec fun add(value: T): Boolean {
        val (prev, cur) = search(value)

        return if (cur != null && value == cur.value) {
            // already in set
            false
        } else {
            val next = TailNode(value, cur)
            val succeeded = prev.tryCASNext(cur, next, false, false)
            if (succeeded) {
                // add succeeded
                true
            } else {
                // retry otherwise
                add(value)
            }
        }
    }

    override tailrec fun remove(value: T): Boolean {
        val (_, cur) = search(value)
        return if (cur == null || value != cur.value) {
            false             // not in set already
        } else {
            val succeeded = cur.tryMarkForDeletion()
            if (succeeded) {
                true          // lazy remove by setting mark in next succeeded
            } else {
                remove(value) // retry otherwise
            }
        }
    }

    override fun contains(value: T): Boolean {
        val (_, cur) = search(value)
        return cur != null && value == cur.value
    }

    override tailrec fun isEmpty(): Boolean {
        val afterHead = head.next

        return when {
            afterHead == null             -> true  // there is nothing after head => empty
            afterHead.isMarkedForDeletion -> {
                propagateForward(head, afterHead)  // remove marked
                isEmpty()                          // and start over
            }
            else                          -> false // not empty
        }
    }
}