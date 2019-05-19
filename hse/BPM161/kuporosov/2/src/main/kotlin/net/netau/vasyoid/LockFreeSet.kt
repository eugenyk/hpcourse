package net.netau.vasyoid

import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicStampedReference
import kotlin.math.absoluteValue
import kotlin.math.sign

private typealias Ref<T> = AtomicStampedReference<Node<T>>

private data class Node<T>(val value: T, val next: Ref<T>)

class LockFreeSet<T : Comparable<T>> {

    private val head = Ref<T>(null, 0)

    private val limit = AtomicInteger(0)

    private fun incStamp(stamp: Int) = if (stamp == 0) 1 else stamp.sign * (stamp.absoluteValue + 1)

    fun add(value: T): Boolean {
        limit.incrementAndGet()
        val newNode = Node(value, Ref(null, 0))
        if (head.compareAndSet(null, newNode, 0, 1)) {
            return true
        }
        while (true) {
            var curRef = head
            while (curRef.reference != null) {
                if (curRef.stamp > 0 && curRef.reference.value.compareTo(value) == 0) {
                    return false
                }
                curRef = curRef.reference.next
            }
            val stamp = curRef.stamp
            if (curRef.compareAndSet(null, newNode, stamp, incStamp(stamp))) {
                return true
            }
        }
    }

    fun remove(value: T): Boolean {
        var curRef = head
        while (curRef.reference != null) {
            val node = curRef.reference
            val stamp = curRef.stamp
            if (node.value.compareTo(value) == 0 && stamp > 0 &&
                curRef.compareAndSet(node, node, stamp, incStamp(-stamp))) {
                return true
            }
            curRef = node.next
        }
        return false
    }

    fun contains(value: T): Boolean {
        val lim = limit.get()
        var cnt = 0
        var curRef = head
        while (cnt++ < lim && curRef.reference != null) {
            if (curRef.reference.value.compareTo(value) == 0 && curRef.stamp > 0) {
                return true
            }
            curRef = curRef.reference.next
        }
        return false
    }

    private fun getElements(): List<Pair<Int, T>> {
        val elements = mutableListOf<Pair<Int, T>>()
        var curRef = head
        while (curRef.reference != null) {
            elements += curRef.stamp to curRef.reference.value
            curRef = curRef.reference.next
        }
        return elements
    }

    fun iterator(): Iterator<T> {
        var prevElements = listOf<Pair<Int, T>>()
        var curElements = getElements()
        while (prevElements != curElements) {
            prevElements = curElements
            curElements = getElements()
        }
        return curElements.filter { it.first > 0 }.map { it.second }.iterator()
    }

    fun isEmpty() = !iterator().hasNext()
}
