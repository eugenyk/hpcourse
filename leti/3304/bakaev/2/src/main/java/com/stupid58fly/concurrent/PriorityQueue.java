package com.stupid58fly.concurrent;

import java.util.Queue;

public interface PriorityQueue<E extends Comparable<E>> extends Queue<E> {
    /**
     * Проверка очереди на пустоту
     *
     * Метод должен быть lock-free (wait-free для уверенных в себе)
     *
     * @return true если очередь пуста, иначе - false
     */
    boolean isEmpty();
}
