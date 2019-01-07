package LockFreePriorityQueue;
import java.util.concurrent.atomic.AtomicReference;
/**
 * Lock-free priority queue
 * @param <E> elements type
 */
public class LockFreePriorityQueue<E extends Comparable<E>> implements PriorityQueue {
    private AtomicReference<LockFreePriorityQueueElement<E>> Head;
    
    public LockFreePriorityQueue()
    {
        Head=new AtomicReference<>(null);
    }
    
    /**
     * Check that queue is empty
     *
     * @return true if queue is empty, else - false
     */
    public boolean isEmpty()
    {
        if(Head.get()==null)
            return true;
        return false;
    }
    
    /**
     * Add element to queue
     *
     * @param value adding element
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
     * Extract element from queue
     *
     * @return element with minimum value
     */
    public E extractMinimum()
    {
        while(true)
        {
			LockFreePriorityQueueElement<E> minElement = Head.get();
            if(minElement==null)
                return null;
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
                    elementAfterMin = minElement.Next.get();
                }
                if(curElement!=null)
                    curElement = curElement.Next.get();
                if(elementBeforeCur!=null)
                    elementBeforeCur = elementBeforeCur.Next.get();
                if(elementAfterCur!=null)
                    elementAfterCur = elementAfterCur.Next.get();
            }
            if(elementBeforeMin==null)
            {
				if(Head.compareAndSet(minElement, elementAfterMin))
				{
					return minElement.Value;
				}
            }
            else
            {
				if(elementBeforeMin.Next.compareAndSet(minElement, elementAfterMin)&&(minElement.Next.compareAndSet(elementAfterMin, null)))
				{
					return minElement.Value;
				}
            }
        }
    }
}