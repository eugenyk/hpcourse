import java.util.Collection;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicReference;

public class LockFreePriorityQueue<E extends Comparable<E>> implements PriorityQueue<E> {
	
	private AtomicReference<E> head = null;
	private AtomicReference<E> tail = null;

	public LockFreePriorityQueue() {
		super();
		// TODO Auto-generated constructor stub
	}

	public boolean add(E arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public E element() {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean offer(E arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public E peek() {
		// TODO Auto-generated method stub
		return null;
	}

	public E poll() {
		// TODO Auto-generated method stub
		return null;
	}

	public E remove() {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean addAll(Collection<? extends E> arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public void clear() {
		// TODO Auto-generated method stub
		
	}

	public boolean contains(Object arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean containsAll(Collection<?> arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public Iterator<E> iterator() {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean remove(Object arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean removeAll(Collection<?> arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean retainAll(Collection<?> arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	public int size() {
		// TODO Auto-generated method stub
		return 0;
	}

	public Object[] toArray() {
		// TODO Auto-generated method stub
		return null;
	}

	public <T> T[] toArray(T[] arg0) {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean isEmpty() {
		// TODO Auto-generated method stub
		return false;
	}

}
