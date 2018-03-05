package lab2;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

	private final Node<T> head;

	public LockFreeSetImpl() {
		head = new Node<>(null);
	}

	private class Node<T> {
		T value;
		AtomicMarkableReference<Node<T>> next;

		Node(T value) {
			this.value = value;
			next = new AtomicMarkableReference<Node<T>>(null, false);
		}
	}

	private class Pair<T> {
		Node<T> prev;
		Node<T> cur;

		Pair(Node<T> prev, Node<T> cur) {
			this.prev = prev;
			this.cur = cur;
		}
	}

	public boolean add(T value) {
		Node<T> toAdd = new Node<>(value);
		while (true) {
			Pair<T> pair = findPosition(value);
			if (value.equals(pair.prev.value) && !pair.prev.next.isMarked()) {
				return false;
			}
			toAdd.next = new AtomicMarkableReference<>(pair.cur, false);
			if (pair.prev.next.compareAndSet(pair.cur, toAdd, false, false)) {
				return true;
			}
		}
	}

	@Override
	public boolean remove(T value) {
		while (true) {
			Pair<T> pair = findPosition(value);
			if (!value.equals(pair.prev.value)) {
				return false;
			}
			
			Node<T> next = pair.prev.next.getReference();
			if(pair.prev.next.compareAndSet(next, next, false, true));
			return true;
		}
	}

	public boolean contains(T value) {
		Pair<T> pair = findPosition(value);
		if (pair.prev == head)
			return false;
		if (pair.prev.value.equals(value))
			return true;
		return false;
	}

	public boolean isEmpty() {
		while (true) {
			while (true) {
				Node<T> nextElem = head.next.getReference();
				if (nextElem == null)
					return true;
				if (!nextElem.next.isMarked())
					return false;
				if (!head.next.compareAndSet(nextElem, nextElem.next.getReference(), false, false)) {
					break;
				}
			}
		}
	}

	public Pair<T> findPosition(T value) {
		while (true) {
			Node<T> prev = head;
			Node<T> cur = head.next.getReference();

			while (true) {
				if (cur == null)
					return new Pair<T>(prev, null);
				if (cur.next.isMarked()) {
					if (!prev.next.compareAndSet(cur, cur.next.getReference(), false, false)) {
						break;
					}
					cur = prev.next.getReference();
					continue;
				}
				if (cur.value.compareTo(value) > 0) {
					return new Pair<>(prev, cur);
				}
				prev = cur;
				cur = cur.next.getReference();
			}
		}
	}
	
}