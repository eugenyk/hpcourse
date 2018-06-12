package ru.spbau.maxim.hw2

import java.util.concurrent.atomic.AtomicMarkableReference

class LockFreeSetImpl<T : Comparable<T>> : LockFreeSet<T> {
    private val tail = Node<T>(null, null)
    private val head = Node<T>(null, tail)

    override fun add(value: T): Boolean {
        while (true) {
            val (prev, node) = findRange(value)

            if (compareWithNode(value, node) == 0) {
                return false
            }
            val newNode = Node(value, node)
            if (prev.next.compareAndSet(node, newNode, false, false)) {
                return true
            }
        }
    }

    override fun remove(value: T): Boolean {
        val isDeletedBuf = booleanArrayOf(false)
        while (true) {
            val (_, node: Node<T>) = findRange(value)
            val nextNode = node.next.get(isDeletedBuf)
            if (compareWithNode(value, node) != 0 || isDeletedBuf[0]) {
                return false
            }
            if (node.next.attemptMark(nextNode, true)) {
                return true
            }
        }
    }

    override fun contains(value: T): Boolean {
        val (_, node: Node<T>) = findRange(value)
        return value == node.value && !node.next.isMarked
    }

    override fun isEmpty(): Boolean {
        val (l, r) = findRangePredicate { node -> !node.next.isMarked }
        return l === head && r === tail
    }

    private fun compareWithNode(value: T, node: Node<T>): Int = when (node) {
        head -> 1
        tail -> -1
        else -> value.compareTo(node.value!!)
    }

    private class Node<T>(val value: T?, nextNode: Node<T>?) {
        val next = AtomicMarkableReference(nextNode, false)
    }

    private fun findRange(value: T): Pair<Node<T>, Node<T>> {
        return findRangePredicate { node -> compareWithNode(value, node) <= 0 }
    }

    //находит первый элемент, подходящий под predicate, и предыдуший (считается, что tail всегда подходит по predicate)
    //возвращает Pair(предыдущий, подходящий элемент)
    //также выполняет helping
    private fun findRangePredicate(predicate: (Node<T>) -> Boolean): Pair<Node<T>, Node<T>> {
        val isDeletedBuf = booleanArrayOf(false)
        loop@ while (true) {
            var prev: Node<T>? = null
            var node = head
            var nextNode = head.next.get(isDeletedBuf)
            while (nextNode != null && (node === head || !predicate(node))) {
                prev = node
                node = nextNode
                nextNode = node.next.get(isDeletedBuf)

                if (isDeletedBuf[0]) {
                    if (!prev.next.compareAndSet(node, nextNode, false, false)) {
                        continue@loop
                    } else {
                        node = prev
                    }
                }
            }

            // l < value <= r
            return Pair(prev!!, node)
        }
    }
}