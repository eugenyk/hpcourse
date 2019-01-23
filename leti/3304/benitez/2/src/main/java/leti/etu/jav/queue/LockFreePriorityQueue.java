package leti.etu.jav.queue;

import java.util.Collection;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.AbstractQueue;
import java.util.NoSuchElementException;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E>{
	
	private AtomicMarkableReference<Node<E>> head = null;
	private AtomicMarkableReference<Node<E>> tail = null;
	private AtomicInteger qsize;
	private class Window
	{
		public Node<E> pred;
		public Node<E> curr;
		public Window(Node<E> p, Node<E> c)
		{
			pred = p;
			curr = c;
		}
	}

	private Window find(E data)
	{
		boolean[] marked = {false};
		Node<E> pred = head.getReference();
		Node<E> curr = pred.next.getReference();
		Node<E> succ = curr.next.get(marked);
		while (true)
		{
			if (marked[0])
			{
				if (pred.next.compareAndSet(curr, succ, false, false))
				{					
					qsize.decrementAndGet();
				}
			}
			else if (succ == null || data.compareTo(curr.data) < 0)
			{
				return new Window(pred, curr);
			}
			pred = curr;
			curr = curr.next.getReference();
			succ = curr.next.get(marked);
		}
	}

	public LockFreePriorityQueue() {
		qsize = new AtomicInteger(0);
		Node<E> t_sentinel = new Node<E>(null);
		Node<E> h_sentinel = new Node<E>(null, t_sentinel);
		this.head = new AtomicMarkableReference<Node<E>>(h_sentinel, false);
		this.tail = new AtomicMarkableReference<Node<E>>(t_sentinel, false);
	}

	public boolean add(E data) {
		Node<E> node = new Node<E>(data);

		while (true)
		{
			Window w = find(data);
			Node<E> pred = w.pred;
			Node<E> curr = w.curr;
			node.next.set(curr, false);
			if (pred.next.compareAndSet(curr, node, false, false))
			{
				qsize.incrementAndGet();
				return true;
			}
		}
	}

	public E element() throws NoSuchElementException {
		boolean[] marked = {false};
		Node<E> first = null;
		Node<E> last = null;
		Node<E> next = null;
		Node<E> succ = null;
		while (true)
		{
			first = head.getReference();
			last = tail.getReference();
			next = first.next.getReference();
			succ = next.next.get(marked);
			if (marked[0])
			{
				if (first.next.compareAndSet(next, succ, false, false))
				{
					qsize.decrementAndGet();
				}
			}
			else if (next == last)
			{
				throw new NoSuchElementException();
			}
			else
			{
				E value = next.data;
				return value;
			}
		}	
	}

	public boolean offer(E data) {
		return this.add(data);
	}

	public E peek() {
		try {
			return this.element();
		} catch (NoSuchElementException e) {
			return null;
		}
	}

	public E poll() {
		try {
			return this.remove();
		} catch (NoSuchElementException e) {
			return null;
		}
	}

	public E remove() throws NoSuchElementException {
		boolean[] marked = {false};
		Node<E> first = null;
		Node<E> last = null;
		Node<E> next = null;
		Node<E> succ = null;
		while (true)
		{
			first = head.getReference();
			last = tail.getReference();
			next = first.next.getReference();
			succ = next.next.get(marked);
			if (marked[0])
			{
				if (first.next.compareAndSet(next, succ, false, false))
				{
					qsize.decrementAndGet();
				}
			}
			else if (next == last)
			{
				throw new NoSuchElementException();
			}
			else
			{
				E value = next.data;
				// mark to remove
				if (next.next.compareAndSet(succ, succ, false, true))
				{
					if (first.next.compareAndSet(next, succ, false, false))
					{
						qsize.decrementAndGet();
					}
					return value;
				}
			}
		}
	}

	public boolean addAll(Collection<? extends E> c) {
		for (E elem : c)
		{
			this.add(elem);
		}
		return true;
	}

	public void clear() {
		while (!this.isEmpty())
		{
			this.poll();
		}
	}

	public Iterator<E> iterator() {
		return null;
	}

	public int size() {
		return qsize.get();
	}

	public boolean isEmpty() {
		boolean[] marked = {false};
		Node<E> first = null;
		Node<E> last = null;
		Node<E> next = null;
		Node<E> succ = null;
		while (true)
		{
			first = head.getReference();
			last = tail.getReference();
			next = first.next.getReference();
			succ = next.next.get(marked);
			if (marked[0])
			{
				if (first.next.compareAndSet(next, succ, false, false))
				{
					qsize.decrementAndGet();
				}
			}
			else
			{
				return (next == last);
			}
		}
	}
}
