package LockFreePriorityQueue;
import java.util.concurrent.atomic.AtomicReference;
import java.util.NoSuchElementException;
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
    /**
     * Head is the first element of linked list that store queue elements and 
     * the last element of queue (elements of queue are stored in reverse order to simplify adding of new element)
     */
    private AtomicReference<LockFreePriorityQueueElement<E>> Head;

    class LockFreePriorityQueueIterator implements Iterator<E>
    {
        LockFreePriorityQueueElement<E> CurElement;

        public LockFreePriorityQueueIterator()
        {
            CurElement=null;
        }

        /**
        * Returns the next element in the iteration
        *
        * @return the next element in the iteration
        */
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

        /**
        * Returns true if the iteration has more elements
        *
        * @return true if the iteration has more elements
        */
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
    @Override
    public boolean isEmpty()
    {
        if(Head.get()==null)
            return true;
        return false;
    }

    /**
     * Inserts the specified element into queue if it is possible to do 
     * so immediately without violating capacity restrictions, returning true upon success 
     * and throwing an OutOfMemoryError if no space is currently available.
     *
     * @param object adding element
     * 
     * @return true
     */
    @Override
    public boolean add(Object object)
    {
        E value=(E)object;
        LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
        LockFreePriorityQueueElement<E> newFirstElement;
        while(true)
        {
			try
			{
				newFirstElement = new LockFreePriorityQueueElement<>(value, oldFirstElement);
			}
			catch(OutOfMemoryError e)
			{
				throw e;
			}
            if(Head.compareAndSet(oldFirstElement, newFirstElement))
                break;
            else
                oldFirstElement = Head.get();
        }
        return true;
    }

    /**
     * Inserts the specified element into this queue if it is possible to do 
     * so immediately without violating capacity restrictions. 
     *
     * @param object adding element
     * 
     * @return true if the element was added to this queue, else false
     */
    @Override
    public boolean offer(Object object)
    {
        E value=(E)object;
        LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
        LockFreePriorityQueueElement<E> newFirstElement;
        while(true)
        {
			try
			{
				newFirstElement = new LockFreePriorityQueueElement<>(value, oldFirstElement);
			}
			catch(OutOfMemoryError e)
			{
				return false;
			}
            if(Head.compareAndSet(oldFirstElement, newFirstElement))
                break;
            else
                oldFirstElement = Head.get();
        }
        return true;
    }

    /**
     * Extract element from queue. Throws an exception if this queue is empty.
     *
     * @return the first element with minimum value
     */
    @Override
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
					if(minElement.Next.compareAndSet(elementAfterMin, null))
						return minElement.Value;
					if(!Head.compareAndSet(elementAfterMin, minElement))
						Head.get().Next=new AtomicReference<>(minElement);
				}
            }
            else
            {
				if(elementBeforeMin.Next.compareAndSet(minElement, elementAfterMin))
				{
					if((minElement.Next.compareAndSet(elementAfterMin, null)))
						return minElement.Value;
					if(!elementBeforeMin.Next.compareAndSet(elementAfterMin, minElement))
						elementBeforeMin.Next.get().Next=new AtomicReference<>(minElement);
				}
            }
		}
    }

    /**
     * Extract element from queue, or returns null if this queue is empty.
     *
     * @return the first element with minimum value, or null if queue is empty.
     */
    @Override
    public E poll()
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
					if(minElement.Next.compareAndSet(elementAfterMin, null))
						return minElement.Value;
					if(!Head.compareAndSet(elementAfterMin, minElement))
						Head.get().Next=new AtomicReference<>(minElement);
				}
            }
            else
            {
				if(elementBeforeMin.Next.compareAndSet(minElement, elementAfterMin))
				{
					if((minElement.Next.compareAndSet(elementAfterMin, null)))
						return minElement.Value;
					if(!elementBeforeMin.Next.compareAndSet(elementAfterMin, minElement))
						elementBeforeMin.Next.get().Next=new AtomicReference<>(minElement);
				}
            }
		}
    }

    /**
     * Retrieves, but does not remove, element from queue. Throws an exception if this queue is empty.
     *
     * @return the first element with minimum value
     */
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

    /**
     * Retrieves, but does not remove, element from queue, or returns null if this queue is empty.
     *
     * @return the first element with minimum value, or null if queue is empty.
     */
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

    /**
     * Removes all of the elements from this collection. The collection will be empty after this method returns.
     */
    @Override
    public void clear()
    {
		Head=new AtomicReference<>(null);;
    }

    /**
     * Unsupported operation
     */
    @Override
    public boolean retainAll(Collection c)
    {
        throw new UnsupportedOperationException();
    }

    /**
     * Unsupported operation
     */
    @Override
    public boolean removeAll(Collection c)
    {
        throw new UnsupportedOperationException();
    }

    /**
     * Adds all of the elements in the specified collection to queue. 
     * The behavior of this operation is undefined if the specified collection is modified while the operation is in progress.
     *
     * @param collection collection containing elements to be added to queue
     * 
     * @return true if this queue changed as a result of the call
     */
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

    /**
     * Returns true if queue contains all of the elements in the specified collection. 
     *
     * @param collection collection to be checked for containment in queue
     * 
     * @return true if this queue contains all of the elements in the specified collection
     */
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

    /**
     * Unsupported operation. 
     */
    @Override
    public boolean remove(Object o)
    {
        throw new UnsupportedOperationException();
    }

    /**
     * Returns an array containing all of the elements in this collection
     *
     * @return an array containing all of the elements in this collection
     */
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

    /**
     * Returns an array containing all of the elements in queue
     * If queue fits in the specified array, it is returned therein. 
     * Otherwise, a new array is allocated with the runtime type of the specified array and 
     * the size of queue.
     * If queue fits in the specified array with room to spare 
     * (i.e., the array has more elements than this collection), the element in the array immediately following 
     * the end of queue is set to null. 
     * (This is useful in determining the length of this collection only if the caller knows that queue 
     * does not contain any null elements.) 
     *
     * @param array the array into which the elements of queue are to be stored, 
     * if it is big enough; otherwise, a new array of the same runtime type is allocated for this purpose.
     *
     * @return an array containing all of the elements in queue
     */
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

    /**
     * Returns an iterator over the elements in this collection.
     *
     * @return iterator in interface Iterable<E>
     */
    @Override
    public Iterator<E> iterator()
    {
        return new LockFreePriorityQueueIterator();
    }

    /**
     * Returns true if queue contains the specified element. 
     * More formally, returns true if and only if queue contains at least one element e 
     * such that (o==null ? e==null : o.equals(e)).
     * 
     * @param object element whose presence in this collection is to be tested
     *
     * @return true if this collection contains the specified element
     */
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

    /**
     * Returns the number of elements in this collection. 
     * If this collection contains more than Integer.MAX_VALUE elements, returns Integer.MAX_VALUE.
     *
     * @return the number of elements in this collection
     */
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
}
