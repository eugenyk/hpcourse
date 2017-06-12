/**
 * Created by YuryKravchenko on 08/06/2017.
 */
public interface LockFreeSet<T extends Comparable<T>> {
    /**
     }
     * Добавить ключ к множеству *
     * Алгоритм должен быть как минимум lock-free *
     *
     *
     */
    boolean add(T value);
    /**
     * Удалить ключ из множества *
     * Алгоритм должен быть как минимум lock-free *
     *
     *
     */
    boolean remove(T value);
    /**
     * Проверка наличия ключа в множестве *
     * Алгоритм должен быть как минимум wait-free *
     *
     *
     */
    boolean contains(T value);
    /**
    * Проверка множества на пустоту *
    * Алгоритм должен быть wait-free (достаточно lock-free, wait-free для сильно уверенных в себе) *
    *
    */
    boolean isEmpty();
}
