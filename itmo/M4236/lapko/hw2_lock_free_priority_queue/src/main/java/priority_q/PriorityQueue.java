package priority_q;

import java.util.Queue;

/**
 * Lock-Free очередь с приоритетами
 *
 * @param <E extends Comparable<E>> Тип элементов
 */
public interface PriorityQueue<E extends Comparable<E>> extends Queue<E> {

    /**
     * Проверка очереди на пустоту
     * <p>
     * Метод должен быть lock-free (wait-free для уверенных в себе)
     *
     * @return true если очередь пуста, иначе - false
     */
    boolean isEmpty();
}
