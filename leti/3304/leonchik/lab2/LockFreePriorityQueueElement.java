package com.leti.lab2;

import java.util.concurrent.atomic.AtomicReference;

public class LockFreePriorityQueueElement<E extends Comparable<E>>
{
	public E Value;
	public AtomicReference<LockFreePriorityQueueElement<E>> Next;

	public LockFreePriorityQueueElement(E value, LockFreePriorityQueueElement<E> next)
	{
		Value = value;
		Next = new AtomicReference<>(next);
	}
}
