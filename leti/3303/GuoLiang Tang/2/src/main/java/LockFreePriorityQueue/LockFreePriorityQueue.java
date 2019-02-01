package LockFreePriorityQueue;
import java.util.NoSuchElementException;
import java.util.concurrent.atomic.AtomicReference;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
/**
 * Lock-free priority queue
 * @param <E> elements type
 */
public class LockFreePriorityQueue<E extends Comparable<E>> implements PriorityQueue {
    private AtomicReference<LockFreePriorityQueueElement<E>> Head;

    class LockFreePriorityQueueIterator implements Iterator<E>
    {
        LockFreePriorityQueueElement<E> CurElement;

        public LockFreePriorityQueueIterator()
        {
            CurElement=null;
        }

        @Override
        public E next()
        {
            if(CurElement==null)
            {
                CurElement=Head.get();
                if(CurElement==null)
                    throw new NoSuchElementException();
                return CurElement.Value;
            }
            if(CurElement.Next.get()==null)
                throw new NoSuchElementException();
            CurElement=CurElement.Next.get();
            return CurElement.Value;
        }

        @Override
        public boolean hasNext()
        {
            if(CurElement==null)
            {
                if(Head.get()==null)
                    return false;
                return true;
            }
            if(CurElement.Next.get()==null)
                return false;
            return true;
        }
    }
    
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
    @Override
    public boolean add(Object value)
    {
        LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
        LockFreePriorityQueueElement<E> newFirstElement = new LockFreePriorityQueueElement<E>((E)value, oldFirstElement);
        while(true)
        {
            newFirstElement.Next = new AtomicReference<>(oldFirstElement);
            if(Head.compareAndSet(oldFirstElement, newFirstElement))
                break;
            else
                oldFirstElement = Head.get();
        }
        return true;
    }

    @Override
    public boolean offer(Object object)
    {
        return add(object);
    }
    
    /**
     * Extract element from queue
     *
     * @return element with minimum value
     */
    public E remove()
    {
        while(true)
        {
			LockFreePriorityQueueElement<E> minElement = Head.get();
            if(minElement==null)
                throw new NoSuchElementException();
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

    @Override
    public E poll()
    {
        try
        {
            return remove();
        }
        catch(NoSuchElementException e)
        {
            return null;
        }
    }

    @Override
    public E element()
    {
        LockFreePriorityQueueElement<E> minElement = Head.get();
        if(minElement==null)
            throw new NoSuchElementException();
        LockFreePriorityQueueElement<E> curElement = minElement.Next.get();
        while(curElement!=null)
        {
            if(curElement.Value.compareTo(minElement.Value) <= 0)
            {
                minElement = curElement;
            }
            curElement = curElement.Next.get();
        }
        return minElement.Value;
    }

    @Override
    public E peek()
    {
        LockFreePriorityQueueElement<E> minElement = Head.get();
        if(minElement==null)
            return null;
        LockFreePriorityQueueElement<E> curElement = minElement.Next.get();
        while(curElement!=null)
        {
            if(curElement.Value.compareTo(minElement.Value) <= 0)
            {
                minElement = curElement;
            }
            curElement = curElement.Next.get();
        }
        return minElement.Value;
    }

    @Override
    public int size()
    {
        if(Head==null)
            return 0;
        LockFreePriorityQueueElement<E> curElement = Head.get();
        int count = 0;
        while(curElement!=null)
        {
            count++;
            if(count==Integer.MAX_VALUE)
                return count;
            curElement = curElement.Next.get();
        }
        return count;
    }

    @Override
    public boolean containsAll(Collection collection)
    {
        for (Iterator iterator = collection.iterator(); iterator.hasNext();)
        {
            E value = (E)iterator.next();
            LockFreePriorityQueueElement<E> curElement = Head.get();
            boolean isValueExist = false;
            while(curElement!=null)
            {
                if(curElement.Value.compareTo(value) == 0)
                {
                    isValueExist = true;
                    break;
                }
                curElement = curElement.Next.get();
            }
            if(!isValueExist)
                return false;
        }
        return true;
    }

    @Override
    public boolean contains(Object object)
    {
        E value = (E)object;
        LockFreePriorityQueueElement<E> curElement = Head.get();
        boolean isValueExist = false;
        while(curElement!=null)
        {
            if(curElement.Value.compareTo(value) == 0)
            {
                isValueExist = true;
                break;
            }
            curElement = curElement.Next.get();
        }
        if(!isValueExist)
            return false;
        return true;
    }

    @Override
    public Iterator<E> iterator()
    {
        return new LockFreePriorityQueueIterator();
    }

    @Override
    public Object[] toArray()
    {
        List<E> result=new ArrayList<E>();
        LockFreePriorityQueueElement<E> curElement = Head.get();
        while(curElement!=null)
        {
            result.add(curElement.Value);
            curElement = curElement.Next.get();
        }
        Collections.reverse(result);//elements of queue are stored in reverse order
        return result.toArray();
    }

    @Override
    public Object[] toArray(Object[] array)
    {
        List<E> result=new ArrayList<E>();
        LockFreePriorityQueueElement<E> curElement = Head.get();
        while(curElement!=null)
        {
            result.add(curElement.Value);
            curElement = curElement.Next.get();
        }
        Collections.reverse(result);//elements of queue are stored in reverse order
        if(result.size()==array.length)
        {
            for(int i=0; i<result.size(); i++)
                array[i]=result.get(i);
            return array;
        }
        else if(result.size()<array.length)
        {
            for(int i=0; i<result.size(); i++)
                array[i]=result.get(i);
            array[result.size()]=null;
            return array;
        }
        return result.toArray();
    }

    @Override
    public boolean remove(Object o)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean addAll(Collection collection)
    {
        if(collection.size()==0)
            return false;
        for (Iterator iterator = collection.iterator(); iterator.hasNext();)
        {
            Object value = iterator.next();
            try
            {
                add(value);
            }
            catch(OutOfMemoryError e)
            {
                throw e;
            }
        }
        return true;
    }

    @Override
    public boolean removeAll(Collection c)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean retainAll(Collection c)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear()
    {
        Head=new AtomicReference<>(null);;
    }
}