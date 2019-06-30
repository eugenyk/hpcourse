package ru.hse.iisuslik

import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import java.util.concurrent.atomic.AtomicStampedReference

/**
 * Lock-Free множество.
 * @param <T> Тип ключей
</T>
 */
class LockFreeSet<T : Comparable<T>> {

    private var currentVersion = AtomicInteger(0)
    private val head = Node(Node<T>(null, null), null)


    /**
     * Проверка множества на пустоту
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @return true если множество пусто, иначе - false
     */
    fun isEmpty(): Boolean {
        return makeSnapshot().isEmpty()
    }

    /**
     * Добавить ключ к множеству
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если value уже существует в множестве, true если элемент был добавлен
     */
    fun add(value: T): Boolean {
        while (true) {
            val (pred, curr) = find(value)
            if (curr.data != null && curr.data == value) {
                return false
            } else {
                val newNode = Node(null, value)
                newNode.next.set(curr, NOT_DELETED)
                val oldVersion = pred.version
                if (pred.next.compareAndSet(curr, newNode, oldVersion * 2, (oldVersion + 1) * 2)) {
                    return true
                }
            }
        }
    }


    /**
     * Удалить ключ из множества
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если ключ не был найден, true если ключ успешно удален
     */
    fun remove(value: T): Boolean {
        while (true) {
            val (pred, curr) = find(value)
            if (curr.data != value) {
                return false
            } else {
                val succ = curr.next.reference
                val currVersion = curr.version
                val predVersion = pred.version
                if (curr.next.compareAndSet(succ, succ, currVersion * 2, (currVersion + 1) * 2 + 1)) {
                    pred.next.compareAndSet(curr, succ, predVersion * 2, (predVersion + 1) * 2)
                    return true
                }
            }
        }
    }


    /**
     * Проверка наличия ключа в множестве
     *
     * Алгоритм должен быть как минимум wait-free
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    operator fun contains(value: T): Boolean {
        var curr = head.next.reference
        while (true) {
            val succ = curr.next.reference
            if (curr.next.stamp % 2 == DELETED) {
                if (curr.data == value) {
                    return false
                }
                curr = succ
            } else {
                val data = curr.data
                if (data == value) {
                    return true
                }
                if (data == null || data > value) {
                    return false
                } else {
                    curr = succ
                }
            }
        }
    }

    /**
     * Возвращает lock-free итератор для множества
     *
     * @return новый экземпляр итератор для множества
     */
    operator fun iterator(): Iterator<T> {
        return makeSnapshot().listIterator()
    }

    fun print() {
        var curr = head.next.reference
        while (true) {
            val data = curr.data
            if (data == null) {
                print("--\n")
                return
            } else {
                print("--$data")
            }
            curr = curr.next.reference
        }
    }

    private fun find(value: T): Pair<Node<T>, Node<T>> {
        while (true) {
            var pred = head
            var curr = pred.next.reference
            while (true) {
                val succ = curr.next.reference
                if (curr.next.stamp % 2 == DELETED) {
                    val predVersion = pred.version
                    if (!pred.next.compareAndSet(curr, succ, predVersion * 2, predVersion * 2)) {
                        break
                    }
                    curr = succ
                } else {
                    val data = curr.data
                    if (data == null || data >= value) {
                        return Pair(pred, curr)
                    } else {
                        pred = curr
                        curr = succ
                    }
                }
            }

        }
    }

    private fun makeSnapshot(): List<T> {
        var first = unSafeSnapshot()
        while (true) {
            val second = unSafeSnapshot()
            if (first == second) {
                return second.map { it.second }
            } else {
                first = second
            }
        }
    }

    private fun unSafeSnapshot(): List<Pair<Int, T>> {
        val snapshot = mutableListOf<Pair<Int, T>>()
        var curr = head.next.reference
        while (true) {
            val data = curr.data
            if (data == null) {
                return snapshot
            } else if (curr.next.stamp % 2 == NOT_DELETED) {
                snapshot.add(Pair(curr.version, data))
            }
            curr = curr.next.reference
        }
    }


    private inner class Node<T>(nextNode: Node<T>?, val data: T?) {
        val next: AtomicStampedReference<Node<T>> = AtomicStampedReference<Node<T>>(nextNode,
            currentVersion.getAndIncrement() * 2)
        val version: Int
            get() = next.stamp / 2
    }


    companion object {
        const val DELETED = 1
        const val NOT_DELETED = 0
    }
}