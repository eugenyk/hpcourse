package priorityQueue;

import java.util.Queue;

/**
 * Lock-Free очередь с приоритетами
 */
public interface ConcurrentPriorityQueue<E extends Comparable<E>> extends Queue<E>  {

    /**
     * Проверка очереди на пустоту
     *
     * Метод должен быть lock-free (wait-free для уверенных в себе)
     *
     * @return true если очередь пуста, иначе - false
     */
    boolean isEmpty();
}
