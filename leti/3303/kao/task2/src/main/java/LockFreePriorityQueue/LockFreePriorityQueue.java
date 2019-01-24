package LockFreePriorityQueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

	private final Element<E> head;
	private final Element<E> tail;
	AtomicInteger sizeQueue = new AtomicInteger(0);

	public LockFreePriorityQueue() {
		// TODO Auto-generated constructor stub
		head = new Element(null, null, false);
		tail = new Element(null, null, false);
		head.setNext(tail, false);

	}

	@Override
	public boolean offer(E key) {
		// TODO Auto-generated method stub
		final Element<E> element = new Element(key, null, false);
		while (true) {
			final PrevCurr<E> p = find(key);
			final Element<E> prevEle = p.prevEle;
			final Element<E> currEle = p.currEle;
			element.setNext(currEle, false);
			if (prevEle.casNext(currEle, element, false, false)) {
				increment_size();
				break;

			}

		}

		return true;
	}

	@Override
	public E poll() {// return top and delete
		// TODO Auto-generated method stub
		final E result;
		while (true) {
			final Element<E> currEle = internalPeek();
			if (currEle == tail) {
				result = null;
				break;
			}
			final Element<E> expeEle = currEle.getNext();
			if (!currEle.casNext(expeEle, currEle, false, true)) {
				continue;

			}
			head.casNext(currEle, expeEle, false, false);
			result = currEle.data;
			decrement_size();
			break;

		}

		return result;
	}

	@Override
	public E peek() {
		// TODO Auto-generated method stub
		Element<E> topEle = internalPeek();
		
		return topEle.data;
	}

	@Override
	public Iterator<E> iterator() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int size() {
		// TODO Auto-generated method stub
		return sizeQueue.get();
	}

	public boolean isEmpty() {
		return peek()== null;
	}
	private Element<E> internalPeek() {
		final Element<E> prevEle = head;
		Element<E> currEle = null;
		Element<E> expeEle = null;
		boolean marked[] = {false};
		
		while (true) {
			currEle = head.getNext();
			if (currEle==tail) {
				return tail;
			}
			expeEle = currEle.getNext(marked);
			if (marked[0]) {
				prevEle.casNext(currEle, expeEle, false, false);
				continue;
				
			}
			return currEle;
					
			
		}
		
	}
	
	

	private void decrement_size() {
		boolean sucess;
		do {
			int tmp = sizeQueue.get();
			sucess = sizeQueue.compareAndSet(tmp, tmp - 1);
		} while (!sucess);

	}

	private void increment_size() {
		boolean sucess;
		do {
			int tmp = sizeQueue.get();
			sucess = sizeQueue.compareAndSet(tmp, tmp + 1);
		} while (!sucess);

	}

	private PrevCurr<E> find(E key) {
		Element<E> prevEle = null;
		Element<E> currEle = null;
		Element<E> expeEle = null;
		retry: while (true) {
			if (head.getNext() == tail) {
				return new PrevCurr(head, tail);
			}

			prevEle = head;
			boolean marked[] = { false };
			while (true) {
				currEle = prevEle.getNext();
				expeEle = currEle.getNext(marked);
				if (marked[0]) {
					if (!prevEle.casNext(currEle, expeEle, false, false)) {
						continue retry;
					}
					continue;
				}

				if (currEle == tail || currEle.data.compareTo(key) >= 0) {
					return new PrevCurr(prevEle, currEle);

				}
				prevEle = currEle;
			}

		}

	}

}

class PrevCurr<E extends Comparable<E>> {
	final Element<E> prevEle;
	final Element<E> currEle;

	PrevCurr(final Element<E> prevEle, final Element<E> currEle) {
		this.prevEle = prevEle;
		this.currEle = currEle;
	}
}

// AtomicMarkableReference The variable is modified or not
class Element<E extends Comparable<E>> extends AtomicMarkableReference<Element<E>> {

	final E data;

	public Element(E currentData, Element<E> nextData, boolean currentMark) {
		super(nextData, currentMark);
		this.data = currentData;
	}

	public void setNext(Element<E> nextData, boolean currentMark) {
		set(nextData, currentMark);
	}

	public Element<E> getNext() {
		return getReference();
	}

	public Element<E> getNext(boolean currentMark[]) {
		return get(currentMark);
	}

	public boolean casNext(Element<E> nextOld, Element<E> nextNew, boolean currentMarkOld, boolean currentMarkNew) {
		return compareAndSet(nextOld, nextNew, currentMarkOld, currentMarkNew);
	}

	public boolean isCurrentMarked() {
		return isMarked();

	}

}
