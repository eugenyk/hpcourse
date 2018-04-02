package ru.spbau.mit.concurrent.set

/**
 * Lock-Free множество.
 * @param <T> Тип ключей
 */
interface LockFreeSet<in T : Comparable<T>> {
    /**
     * Добавить ключ к множеству
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если value уже существует в множестве, true если элемент был добавлен
     */
    fun add(value: T): Boolean

    /**
     * Удалить ключ из множества
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если ключ не был найден, true если ключ успешно удален
     */
    fun remove(value: T): Boolean

    /**
     * Проверка наличия ключа в множестве
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    fun contains(value: T): Boolean

    /**
     * Проверка множества на пустоту
     *
     * Алгоритм должен быть wait-free (достаточно lock-free, wait-free для сильно уверенных в себе)
     *
     * @return true если множество пусто, иначе - false
     */
    fun isEmpty(): Boolean
}