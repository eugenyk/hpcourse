package com.leti.lab2;

import java.util.Collection;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Lock-free priority queue
 *
 * @param <E> elements type
 */
public class LockFreePriorityQueue<E extends Comparable<E>> implements PriorityQueue<E>
{
	private AtomicReference<LockFreePriorityQueueElement<E>> Head;

	public LockFreePriorityQueue()
	{
		Head = new AtomicReference<>(null);
	}

	@Override
	public boolean isEmpty()
	{
		return Head.get() == null;
	}

	@Override
	public boolean add(final E element)
	{
		LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
		LockFreePriorityQueueElement<E> newFirstElement;
		while (true)
		{
			newFirstElement = new LockFreePriorityQueueElement<>(element, oldFirstElement);

			if (Head.compareAndSet(oldFirstElement, newFirstElement))
				break;
			else
				oldFirstElement = Head.get();
		}
		return true;
	}

	@Override
	public boolean offer(final E element)
	{
		LockFreePriorityQueueElement<E> oldFirstElement = Head.get();
		LockFreePriorityQueueElement<E> newFirstElement;
		while (true)
		{

			newFirstElement = new LockFreePriorityQueueElement<>(element, oldFirstElement);

			if (Head.compareAndSet(oldFirstElement, newFirstElement))
				break;
			else
				oldFirstElement = Head.get();
		}
		return true;
	}

	@Override
	public E remove()
	{
		while (true)
		{
			LockFreePriorityQueueElement<E> minElement = Head.get();
			if (minElement == null)
				throw new NoSuchElementException();
			LockFreePriorityQueueElement<E> elementBeforeMin = null;
			LockFreePriorityQueueElement<E> elementAfterMin = minElement.Next.get();
			LockFreePriorityQueueElement<E> curElement = elementAfterMin;
			LockFreePriorityQueueElement<E> elementBeforeCur = minElement;
			LockFreePriorityQueueElement<E> elementAfterCur = null;
			if (curElement != null)
				elementAfterCur = curElement.Next.get();
			while (curElement != null)
			{
				if (curElement.Value.compareTo(minElement.Value) <= 0)
				{
					minElement = curElement;
					elementBeforeMin = elementBeforeCur;
					elementAfterMin = minElement.Next.get();
				}
				curElement = curElement.Next.get();
				if (elementBeforeCur != null)
					elementBeforeCur = elementBeforeCur.Next.get();
				if (elementAfterCur != null)
					elementAfterCur = elementAfterCur.Next.get();
			}
			if (elementBeforeMin == null)
			{
				if (Head.compareAndSet(minElement, elementAfterMin))
				{
					if (minElement.Next.compareAndSet(elementAfterMin, null))
						return minElement.Value;
					if (!Head.compareAndSet(elementAfterMin, minElement))
						Head.get().Next = new AtomicReference<>(minElement);
				}
			}
			else
			{
				if (elementBeforeMin.Next.compareAndSet(minElement, elementAfterMin))
				{
					if ((minElement.Next.compareAndSet(elementAfterMin, null)))
						return minElement.Value;
					if (!elementBeforeMin.Next.compareAndSet(elementAfterMin, minElement))
						elementBeforeMin.Next.get().Next = new AtomicReference<>(minElement);
				}
			}
		}
	}

	@Override
	public E poll()
	{
		while (true)
		{
			LockFreePriorityQueueElement<E> minElement = Head.get();
			if (minElement == null)
				return null;
			LockFreePriorityQueueElement<E> elementBeforeMin = null;
			LockFreePriorityQueueElement<E> elementAfterMin = minElement.Next.get();
			LockFreePriorityQueueElement<E> curElement = elementAfterMin;
			LockFreePriorityQueueElement<E> elementBeforeCur = minElement;
			LockFreePriorityQueueElement<E> elementAfterCur = null;
			if (curElement != null)
				elementAfterCur = curElement.Next.get();
			while (curElement != null)
			{
				if (curElement.Value.compareTo(minElement.Value) <= 0)
				{
					minElement = curElement;
					elementBeforeMin = elementBeforeCur;
					elementAfterMin = minElement.Next.get();
				}
				curElement = curElement.Next.get();
				if (elementBeforeCur != null)
					elementBeforeCur = elementBeforeCur.Next.get();
				if (elementAfterCur != null)
					elementAfterCur = elementAfterCur.Next.get();
			}
			if (elementBeforeMin == null)
			{
				if (Head.compareAndSet(minElement, elementAfterMin))
				{
					if (minElement.Next.compareAndSet(elementAfterMin, null))
						return minElement.Value;
					if (!Head.compareAndSet(elementAfterMin, minElement))
						Head.get().Next = new AtomicReference<>(minElement);
				}
			}
			else
			{
				if (elementBeforeMin.Next.compareAndSet(minElement, elementAfterMin))
				{
					if ((minElement.Next.compareAndSet(elementAfterMin, null)))
						return minElement.Value;
					if (!elementBeforeMin.Next.compareAndSet(elementAfterMin, minElement))
						elementBeforeMin.Next.get().Next = new AtomicReference<>(minElement);
				}
			}
		}
	}

	@Override
	public E element()
	{
		LockFreePriorityQueueElement<E> minElement = Head.get();
		if (minElement == null)
			throw new NoSuchElementException();
		LockFreePriorityQueueElement<E> curElement = minElement.Next.get();
		while (curElement != null)
		{
			if (curElement.Value.compareTo(minElement.Value) <= 0)
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
		if (minElement == null)
			return null;
		LockFreePriorityQueueElement<E> curElement = minElement.Next.get();
		while (curElement != null)
		{
			if (curElement.Value.compareTo(minElement.Value) <= 0)
			{
				minElement = curElement;
			}
			curElement = curElement.Next.get();
		}
		return minElement.Value;
	}

	@Override
	public void clear()
	{
		Head = new AtomicReference<>(null);
	}

	@Override
	public boolean retainAll(Collection collection)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean removeAll(Collection collection)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean addAll(Collection collection)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean containsAll(Collection collection)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public E[] toArray(Object[] array)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public Iterator<E> iterator()
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean contains(Object object)
	{
		E value = (E) object;
		LockFreePriorityQueueElement<E> curElement = Head.get();
		boolean isValueExist = false;
		while (curElement != null)
		{
			if (curElement.Value.compareTo(value) == 0)
			{
				isValueExist = true;
				break;
			}
			curElement = curElement.Next.get();
		}
		return isValueExist;
	}

	@Override
	public int size()
	{
		if (Head == null)
			return 0;
		LockFreePriorityQueueElement<E> curElement = Head.get();
		int count = 0;
		while (curElement != null)
		{
			count++;
			if (count == Integer.MAX_VALUE)
				return count;
			curElement = curElement.Next.get();
		}
		return count;
	}
}
