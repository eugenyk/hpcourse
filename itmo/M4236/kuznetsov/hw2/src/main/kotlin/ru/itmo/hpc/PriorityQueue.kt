package ru.itmo.hpc

import java.util.*

/**
 * Lock-Free очередь с приоритетами
 * @param <E> Тип элементов
 */
interface PriorityQueue<E: Comparable<E>> : Queue<E> {

    /**
     * Проверка очереди на пустоту
     *
     * Метод должен быть lock-free (wait-free для уверенных в себе)
     *
     * @return true если очередь пуста, иначе - false
     */
    override fun isEmpty(): Boolean
}