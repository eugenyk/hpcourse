package lockFreePriorityQueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class PriorityQueueImpl<E extends Comparable<? super E>> extends AbstractQueue<E> implements PriorityQueue<E> {
	
	private AtomicReference<Node<E>> head = new AtomicReference<Node<E>>(null);
	private AtomicInteger size = new AtomicInteger(0);
	
	@Override
	public boolean offer(E e) {
		while (true) {
			Node<E> h = head.get(),
					next = h == null ? null : h.next.get();

			while (h != null && next != null
					&& !(e.compareTo(h.data) >= 0
					&& e.compareTo(next.data) < 0))
			{
				h = next;
				next = next.next.get();
			}
			if (h == null) {
				if (head.compareAndSet(null, new Node<E>(e)))
				{
					size.incrementAndGet();
					return true;
				}
				continue;
			}

			if (h != null && next == null) {
				if(e.compareTo(h.data) >= 0
						&& h.next.compareAndSet(null, new Node<E>(e)))
				{
					size.incrementAndGet();
					return true;
				}
				
				if(e.compareTo(h.data) < 0
						&& head.compareAndSet(h, new Node<E>(e, h)))
				{
					size.incrementAndGet();
					return true;
				}
				continue;
			}

			if (e.compareTo(h.data) >= 0
					&& e.compareTo(next.data) < 0) {
				if (h.next.compareAndSet(next, new Node<E>(e, next)))
				{
					size.incrementAndGet();
					return true;
				}
				continue;
			}
		}
	}

	@Override
	public E poll() {
		Node<E> result;
		do {
			result = head.get();
		} while (result != null && !head.compareAndSet(result, result.next.get()));
		
		if (result != null) {
			size.decrementAndGet();
			return result.data;
		}
		
		return null;
	}

	@Override
	public E peek() {
		Node<E> head = this.head.get();
		if (head == null)
			return null;
		return head.data;
	}

	

	@Override
	public int size() {
		return size.get();
	}

	@Override
	public boolean isEmpty() {
		return head.get() == null;
	}
	
	
	@Override
	public Iterator<E> iterator() {
		// TODO Auto-generated method stub
		return null;
	}

}
