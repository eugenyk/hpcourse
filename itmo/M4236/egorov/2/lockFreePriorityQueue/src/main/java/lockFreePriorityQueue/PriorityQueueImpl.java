package lockFreePriorityQueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class PriorityQueueImpl<E extends Comparable<? super E>> extends AbstractQueue<E> implements PriorityQueue<E> {
	
	private final AtomicMarkableReference<Node<E>> head = new AtomicMarkableReference<Node<E>>(null, true);
	private final AtomicInteger size = new AtomicInteger(0);
	
	private void findTrueHead() {
		
		Node<E> res = head.getReference();

		if (head.isMarked() || res == null) {
			return;
		}
		
		Node<E> next = res.next.getReference();;
		
		if (res.next.compareAndSet(next, next, true, false)) {
			head.compareAndSet(res, next, false, true);
		}
	}
	
	@Override
	public boolean offer(E e) {
		while (true) {
			
			findTrueHead();
			
			Node<E> hNode = head.getReference();
			
			if (hNode == null) {
				if (head.compareAndSet(null, new Node<E>(e), true, true)) {
					size.incrementAndGet();
					return true;
				}
				continue;
			}

			if(e.compareTo(hNode.data) < 0) {
				if (head.compareAndSet(hNode, new Node<E>(e, hNode), true, true)) {
					size.incrementAndGet();
					return true;
				}
				continue;
			}
			
			Node<E> nextNode = hNode.next.getReference();

			while (nextNode != null
					&& e.compareTo(nextNode.data) >= 0)
			{
				hNode = nextNode;
				nextNode = nextNode.next.getReference();
			}

			if (nextNode == null) {
				if(hNode.next.compareAndSet(null, new Node<E>(e), true, true)) {
					size.incrementAndGet();
					return true;
				}
				continue;
			}

			if (hNode.next.compareAndSet(nextNode, new Node<E>(e, nextNode), true, true)) {
				size.incrementAndGet();
				return true;
			}
		}
	}

	@Override
	public E poll() {
		Node<E> result;
		
		do {
			findTrueHead();
			result = head.getReference();
		} while (result != null && !head.compareAndSet(result, result, true, false));
		
		if (result != null) {
			size.decrementAndGet();
			return result.data;
		}
		
		return null;
	}

	@Override
	public E peek() {
		findTrueHead();
		Node<E> head = this.head.getReference();
		if (head == null)
			return null;
		return head.data;
	}

	

	@Override
	public int size() {
		int s = size.get();
		return s < 0 ? 0 : s;
	}

	@Override
	public boolean isEmpty() {
		findTrueHead();
		return head.getReference() == null;
	}
	
	
	@Override
	public Iterator<E> iterator() {
		// TODO Auto-generated method stub
		return null;
	}

}
