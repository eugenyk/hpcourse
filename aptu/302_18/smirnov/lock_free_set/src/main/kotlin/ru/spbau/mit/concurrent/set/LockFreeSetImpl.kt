package ru.spbau.mit.concurrent.set

import java.util.concurrent.atomic.AtomicMarkableReference

private typealias NodeRef<T> = AtomicMarkableReference<Node<T>>

private sealed class Node<T>(open val value: T?, next: Node<T>?) {
    val next = NodeRef(next, false)
}

private class HeadNode<T>(next: Node<T>? = null): Node<T>(null, next)

private class TailNode<T>(override val value: T, next: Node<T>?): Node<T>(null, next)

private fun <T> propagateForward(cur: Node<T>, markedNext: Node<T>) =
        cur.next.compareAndSet(markedNext, markedNext.next.reference, false, false)

class LockFreeSetImpl<in T : Comparable<T>>: LockFreeSet<T> {
    private val head = HeadNode<T>()

    private fun search(value: T): Pair<Node<T>,Node<T>?> {
        fun helper(prev: Node<T>, curRef: NodeRef<T>): Pair<Node<T>,Node<T>?> {
            val cur = curRef.reference

            return@helper when {
                curRef.isMarked -> {
                    propagateForward(
                            prev,
                            cur
                    )                     // remove marked
                    search(value)         // and start over
                }
                cur == null || (cur is TailNode && value <= cur.value) ->
                    Pair(prev, cur)       // we have either met the end
                                          // or found the desired position
                else ->
                    helper(cur, cur.next) // otherwise simply take the next node
            }
        }

        return helper(head, head.next)
    }

    override fun add(value: T): Boolean {
        val (prev, cur) = search(value)
        return if (cur != null && value == cur.value) {
            false          // already in set
        } else {
            val next = TailNode(value, cur)
            val succeeded = prev.next.compareAndSet(cur, next, false, false)
            if (succeeded) {
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
        val (_, cur) = search(value)
        return cur != null && value == cur.value
    }

    override fun isEmpty(): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

}