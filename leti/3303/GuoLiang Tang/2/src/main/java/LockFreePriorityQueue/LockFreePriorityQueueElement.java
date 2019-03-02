package LockFreePriorityQueue;
import java.util.concurrent.atomic.AtomicReference;
/**
 * Элемент lock-free очереди с приоритетами
 * @param <E> Тип элементов
 */
public class LockFreePriorityQueueElement<E extends Comparable<E>> {
    public E Value;
    public AtomicReference<LockFreePriorityQueueElement<E>> Next;
    public LockFreePriorityQueueElement(E value, LockFreePriorityQueueElement<E> next)
    {
        Value=value;
        Next=new AtomicReference<>(next);
    }
}