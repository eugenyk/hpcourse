package ru.hse.spb.kazakov

import java.lang.IllegalStateException
import java.util.concurrent.atomic.AtomicMarkableReference


/**
 * Lock-Free множество.
 * @param <T> Тип ключей
 */
class LockFreeSet<T : Comparable<T>> : Iterable<T> {
    private val tail = BoundNode<T>()
    private val head = BoundNode(tail)

    /**
     * Lock-free добавление ключа к множеству.
     *
     * @param value значение ключа
     * @return false если value уже существует в множестве, true если элемент был добавлен
     */
    fun add(value: T): Boolean {
        while (true) {
            val (lower, ceiling) = find(value)
            if (ceiling != tail && ceiling.value == value) {
                return false
            }
            if (lower.casNext(ceiling, ValueNode(value, ceiling))) {
                return true
            }
        }
    }

    /**
     * Lock-free удаление ключа из множества.
     *
     * @param value значение ключа
     * @return false если ключ не был найден, true если ключ успешно удален
     */
    fun remove(value: T): Boolean {
        while (true) {
            val (lower, ceiling) = find(value)
            if (ceiling == tail || ceiling.value != value) {
                return false
            }
            val next = ceiling.next
            if (ceiling.casRemoveMark(next)) {
                lower.casNext(ceiling, next)
                return true
            }
        }
    }

    /**
     * Wait-free проверка наличия ключа в множестве.
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    fun contains(value: T): Boolean {
        var current: Node<T> = head.next

        while (current != tail) {
            if (current.value == value && !current.isRemoved) {
                return true
            }
            current = current.next
        }

        return false
    }

    /**
     * Lock-free проверка множества на пустоту.
     *
     * @return true если множество пусто, иначе - false
     */
    fun isEmpty() = makeAtomicSnapshot().isEmpty()

    /**
     * Возвращает lock-free итератор для множества.
     */
    override fun iterator(): Iterator<T> = makeAtomicSnapshot().iterator()

    private fun find(value: T): Pair<Node<T>, Node<T>> {
        while (true) {
            var lower: Node<T> = head
            var ceiling = head.next
            var lowerNext = ceiling

            while (ceiling != tail && (ceiling.isRemoved || ceiling.value < value)) {
                if (!ceiling.isRemoved) {
                    lower = ceiling
                    lowerNext = ceiling.next
                }
                ceiling = ceiling.next
            }

            if (lower.casNext(lowerNext, ceiling)) {
                if (ceiling.isRemoved) {
                    continue
                }
                return Pair(lower, ceiling)
            }
        }
    }

    private fun makeAtomicSnapshot(): List<T> {
        var firstSnapshot = makeSnapshot()
        var secondSnapshot = makeSnapshot()

        while (firstSnapshot != secondSnapshot) {
            firstSnapshot = makeSnapshot()
            secondSnapshot = makeSnapshot()
        }

        return firstSnapshot.map { it.value }
    }

    private fun makeSnapshot(): List<Node<T>> {
        var current: Node<T> = head.next
        val values = mutableListOf<Node<T>>()

        while (current != tail) {
            if (!current.isRemoved) {
                values.add(current)
            }
            current = current.next
        }

        return values
    }

    private abstract class Node<T : Comparable<T>> {
        private val nextRef: AtomicMarkableReference<Node<T>>

        constructor() {
            nextRef = AtomicMarkableReference(this, false)
        }

        constructor(next: Node<T>) {
            nextRef = AtomicMarkableReference(next, false)
        }

        val next: Node<T>
            get() = nextRef.reference

        val isRemoved: Boolean
            get() = nextRef.isMarked

        abstract val value: T

        fun casNext(currentNext: Node<T>, newNext: Node<T>) = nextRef.compareAndSet(currentNext, newNext, false, false)

        fun casRemoveMark(currentNext: Node<T>) = nextRef.compareAndSet(currentNext, currentNext, false, true)
    }

    private class ValueNode<T : Comparable<T>>(override val value: T, next: Node<T>) : Node<T>(next)

    private class BoundNode<T : Comparable<T>> : Node<T> {
        constructor()

        constructor(next: Node<T>) : super(next)

        override val value: T
            get() = throw IllegalStateException("Bound node value queried")
    }
}