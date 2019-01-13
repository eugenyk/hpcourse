package ru.itmo.hpc

import java.util.*
import java.util.concurrent.atomic.AtomicMarkableReference

/**
 * Implementation based on the concepts of Harris list
 * @link https://timharris.uk/papers/2001-disc.pdf
 */
class LockFreePriorityQueue<E: Comparable<E>>: AbstractQueue<E>(), PriorityQueue<E> {

    override fun offer(e: E): Boolean {

        while (true) {
            val (predecessor, current) = getInsertPosition(e)

            if (predecessor.compareAndSetNext(
                            current,
                            Node(e, current),
                            false,
                            false)) {
                return true
            }
        }
    }

    override fun iterator(): MutableIterator<E> {
        TODO("not implemented")
    }

    override val size: Int
        get() = TODO("not implemented") //To change initializer of created properties use File | Settings | File Templates.

    override fun peek(): E? {
        return getInsertPosition().current.data
    }

    /**
     * It does not physically remove any node from queue,
     * but marks node that follows *after* the first present element.
     *
     * Therefore, set mark means that *previous* node must not be considered
     * in further operations.
     */
    override fun poll(): E? {
        while (true) {
            val appropriateNode = getInsertPosition().current
            if (appropriateNode == TAIL)
                return null
            val next = appropriateNode.nextNode
            if (appropriateNode.compareAndSetNext(next, next, false, true)) {
                return appropriateNode.data
            }
        }
    }

    override fun isEmpty(): Boolean {
        val (prev, curr) = getInsertPosition()
        return (prev == HEAD && curr == TAIL)
    }

    inner class Node<E: Comparable<E>>(
            val data: E? = null, // Only for HEAD and TAIL
            val next: Node<E>? = null // Only for TAIL
    ): Comparable<Node<E>> {

        private val nextAtomicReference: AtomicMarkableReference<Node<E>?> = AtomicMarkableReference(next, false)

        val nextNode get() = nextAtomicReference.reference!!

        val isNextMarked get() = nextAtomicReference.isMarked

        override fun compareTo(other: Node<E>): Int =
                when (other) {
                    HEAD -> 1
                    TAIL -> -1
                    else -> data!!.compareTo(other.data!!)
                }

        operator fun compareTo(other: E): Int =
                when (other) {
                    HEAD -> 1
                    TAIL -> -1
                    else -> data!!.compareTo(other)
                }

        fun compareAndSetNext(
                expectedReference: Node<E>,
                newReference: Node<E>,
                expectedMark: Boolean = false,
                newMark: Boolean = false
        ) = nextAtomicReference.compareAndSet(expectedReference, newReference, expectedMark, newMark)
    }

    data class SequentialPair<E: Comparable<E>>(
            val predecessor: LockFreePriorityQueue<E>.Node<E>,
            val current: LockFreePriorityQueue<E>.Node<E>
    )

    /**
     * Get pair of consequent nodes to insert new element right between them
     */
    private fun getInsertPosition(e: E? = null): SequentialPair<E> {
        var previousNode = HEAD
        var currentNode = HEAD.nextNode
        var nextNode: Node<E>

        while (true) {
            var temporalNode = HEAD
            var temporalNextNode = HEAD.nextNode

            do {
                if (!temporalNode.isNextMarked) {
                    previousNode = temporalNode
                    currentNode = temporalNextNode
                }
                temporalNode = temporalNode.nextNode
                if (temporalNode === TAIL) {
                    break
                }
                temporalNextNode = temporalNode.nextNode
            } while (temporalNode.isNextMarked || (e != null && temporalNode <= e))

            nextNode = temporalNode

            if (previousNode.compareAndSetNext(currentNode, nextNode, false, false)) {
                if (nextNode != TAIL && nextNode.isNextMarked) {
                    continue
                } else {
                    return SequentialPair(previousNode, nextNode)
                }
            }
        }
    }

    private val TAIL: Node<E> = Node()
    private val HEAD: Node<E> = Node(next = TAIL)
}