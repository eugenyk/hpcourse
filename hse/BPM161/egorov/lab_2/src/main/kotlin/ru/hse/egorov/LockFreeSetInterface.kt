package ru.hse.egorov

/**
 * Lock-Free множество.
 * @param <T> Тип ключей
</T> */
interface LockFreeSetInterface<T : Comparable<T>> {


    /**
     * Проверка множества на пустоту
     *
     * Алгоритм должен быть как минимум lock-free
     *
     * @return true если множество пусто, иначе - false
     */
    val isEmpty: Boolean

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
     * Алгоритм должен быть как минимум wait-free
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    operator fun contains(value: T): Boolean

    /**
     * Возвращает lock-free итератор для множества
     *
     * @return новый экземпляр итератор для множества
     */
    operator fun iterator(): Iterator<T>
}