package lockfree

import java.util.concurrent.atomic.AtomicMarkableReference

typealias LockFreeNode<T> = AtomicMarkableReference<LockFreeSet.Node<T>>

class LockFreeSet<T> : ILockFreeSet<T>
        where T : Comparable<T> {

    class Node<T>(val value: T?, var next: LockFreeNode<T>)

    private val tail = LockFreeNode<T>(null, false)
    private var head = LockFreeNode(Node(null, tail), false)

    override val isEmpty: Boolean
        get() {
            val mark = booleanArrayOf(false)
            var cur = head.reference.next.get(mark)
            while (cur != null) {
                if (!mark[0])
                    return false
                cur = cur.next.get(mark)
            }
            return true
        }

    override fun add(value: T): Boolean {
        while (true) {
            var cur = head
            val curMark = booleanArrayOf(false)
            var curNode = cur.reference
            val nextMark = booleanArrayOf(false)
            var nextNode = curNode.next.get(nextMark)
            while (nextNode != null) {
                if (nextNode.value == value && !nextMark[0]) {
                    return false
                }
                cur = curNode.next
                curNode = nextNode
                curMark[0] = nextMark[0]
                nextNode = curNode.next.get(nextMark)
            }
            if (cur.compareAndSet(
                    curNode,
                    Node(curNode.value, LockFreeNode(Node(value, tail), false)),
                    curMark[0],
                    curMark[0]
                )
            ) {
                return true
            }
        }
    }

    override fun remove(value: T): Boolean {
        val mark = booleanArrayOf(false)
        var cur = head.reference.next
        var curNode = cur.get(mark)
        while (curNode != null) {
            while (curNode.value == value && !mark[0]) {
                if (cur.compareAndSet(curNode, curNode, false, true)) {
                    return true
                }
                curNode = cur.get(mark)
            }
            cur = curNode.next
            curNode = cur.get(mark)
        }
        return false
    }

    override operator fun contains(value: T): Boolean {
        val mark = booleanArrayOf(false)
        var curNode = head.reference.next.get(mark)
        while (curNode != null) {
            if (curNode.value == value && !mark[0]) {
                return true
            }
            curNode = curNode.next.get(mark)
        }
        return false
    }

    override operator fun iterator(): Iterator<T> {
        var curSeq = mutableListOf<Node<T>>()
        var prevSeq = mutableListOf<Node<T>>()
        do {
            curSeq.clear()
            val mark = booleanArrayOf(false)
            var cur = head.reference.next.get(mark)
            while (cur != null) {
                if (!mark[0]) {
                    curSeq.add(cur)
                }
                cur = cur.next.get(mark)
            }
            curSeq = prevSeq.also { prevSeq = curSeq }
        } while (curSeq != prevSeq)
        return curSeq.map { it.value!! }.iterator()
    }
}
