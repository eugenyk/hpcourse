package ru.hse.nikiforovskaya.set

import java.util.concurrent.atomic.AtomicMarkableReference

class ListNode<T>(private val key: T) {

    companion object {
        val UNMARKED = false
        val MARKED = true
    }

    val next : AtomicMarkableReference<ListNode<T>?> = AtomicMarkableReference(null, UNMARKED)

    fun nextNode() : ListNode<T>? {
        return next.reference
    }

    fun setNextNode(next : ListNode<T>?) : Unit {
        this.next.set(next, UNMARKED)
    }

    fun isMarked() : Boolean {
        return next.isMarked
    }

    fun getKey() : T {
        return key
    }
}