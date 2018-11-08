package LockFreePriorityQueue;
import java.util.concurrent.atomic.AtomicReference;
/**
 * Lock-free очередь с приоритетами
 * @param <E> Тип элементов
 */
public class LockFreePriorityQueue<E extends Comparable<E>> implements PriorityQueue {
    private AtomicReference<LockFreePriorityQueueElement<E>> Head;
    
    public LockFreePriorityQueue()
    {
        Head=new AtomicReference<>(null);
    }
    
    /**
     * Проверка очереди на пустоту
     *
     * Метод должен быть lock-free (wait-free для уверенных в себе)
     *
     * @return true если очередь пуста, иначе - false
     */
    public boolean isEmpty()
    {
        if(Head.get()==null)
            return true;
        return false;
    }
    
    /**
     * Добавление элемента в очередь
     *
     * @param value добавляемый элемент
     */
    public void insert(E value)
    {
        LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
        LockFreePriorityQueueElement<E> newFirstElement = new LockFreePriorityQueueElement<>(value, oldFirstElement);
        while(true)
        {
            newFirstElement.Next = new AtomicReference<>(oldFirstElement);
            if(Head.compareAndSet(oldFirstElement, newFirstElement))
                break;
            else
                oldFirstElement = Head.get();
        }
    }
    
    /**
     * Извлечение элемента из очереди
     *
     * @return элемент с наименьшим значением
     */
    public E extractMinimum()
    {
        while(true)
        {
            if(Head.get()==null)
                return null;
            LockFreePriorityQueueElement<E> minElement = Head.get();
            LockFreePriorityQueueElement<E> elementBeforeMin = null;
            LockFreePriorityQueueElement<E> elementAfterMin = minElement.Next.get();
            LockFreePriorityQueueElement<E> curElement = elementAfterMin;
            LockFreePriorityQueueElement<E> elementBeforeCur = minElement;
            LockFreePriorityQueueElement<E> elementAfterCur = null;
            if(curElement!=null)
                elementAfterCur = curElement.Next.get();
            while(curElement!=null)
            {
                if(curElement.Value.compareTo(minElement.Value) <= 0)
                {
                    minElement = curElement;
                    elementBeforeMin = elementBeforeCur;
                    elementAfterMin = elementAfterCur;
                }
                if(curElement!=null)
                    curElement = curElement.Next.get();
                if(elementBeforeCur!=null)
                    elementBeforeCur = elementBeforeCur.Next.get();
                if(elementAfterCur!=null)
                    elementAfterCur = elementAfterCur.Next.get();
            }
            AtomicReference<LockFreePriorityQueueElement<E>> elementBeforeMinNext;
            if(elementBeforeMin==null)
            {
                elementBeforeMinNext=Head;
            }
            else
            {
                elementBeforeMinNext=elementBeforeMin.Next;
            }
            if(elementBeforeMinNext.compareAndSet(minElement, elementAfterMin))
            {
                return minElement.Value;
            }
        }
    }
}