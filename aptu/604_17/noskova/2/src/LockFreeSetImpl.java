/* based on https://www.microsoft.com/en-us/research/wp-content/uploads/2001/10/2001-disc.pdf */

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
	
	private class Node implements Comparable<Node> {
		Node(T value) {
			this.value = value;
			next = new AtomicMarkableReference<>(null, false); /* A mode is marked if and only if its next field is marked */
		}

		Node(T value, Node next) {
			this.value = value;
			this.next = new AtomicMarkableReference<>(next, false);
		}

		@Override
		public int compareTo(Node node) {
			if (this.value == null || node.value == null)
				return -1;
			return this.value.compareTo(node.value);
		}

		T value;
		AtomicMarkableReference<Node> next;
	}

	private class Pair<T> {
		Pair(T first, T second) {
			this.first = first;
			this.second = second;
		}
		
		T first;
		T second;
	}

	public LockFreeSetImpl() {
		this.tail = new Node(null);
		this.head = new Node(null, tail);
	}
	
	/* This is for add function (from article) */
	Pair<Node> search(T value) {
		Node left_node = head;
		Node left_node_next = null;
		Node right_node;
		do {
			Node t = head;
			Node t_next = head.next.getReference();
			/* 1: Find left_node and right_node */
			do {
				if (!t.next.isMarked()) {
					left_node = t;
					left_node_next = t_next;
				}
				t = t_next;
				if (t == tail)
					break;
				t_next = t.next.getReference();
			} while (t.next.isMarked() || (t.value.compareTo(value) < 0)); 
			right_node = t;
			/* 2: Check nodes are adjacent */
			if (left_node_next == right_node) 
				if (right_node != tail && right_node.next.isMarked())
					continue; 
				else
					return new Pair<Node>(left_node, right_node);
			/* 3: Remove one or more marked nodes */
			if (left_node.next.compareAndSet(left_node_next, right_node, false, false))
				if (right_node != tail && right_node.next.isMarked())
					continue;
				else
					return new Pair<Node>(left_node, right_node); 
		} while (true);
	}
	
	@Override
	public boolean add(T value) {
		Node node = new Node(value);
		do {
			Pair<Node> neighbours = search(value);
			
			/* Check if exists */
			if (neighbours.second != tail && neighbours.second.value.compareTo(value) == 0)
				return false;
			
			node.next = new AtomicMarkableReference<>(neighbours.second, false);

			if (neighbours.first.next.compareAndSet(neighbours.second, node, false, false)) {
				return true;
			}
		}while(true);
	}

	@Override
	public boolean remove(T value) {
		Node left_node;
		Node right_node_next;
		Node right_node;
		do {
			Pair<Node> neighbours = search(value);
			left_node = neighbours.first;
			right_node = neighbours.second;

			/* Check if not exists */
			if (right_node != tail && right_node.value.compareTo(value) != 0)
				return false;
			right_node_next = right_node.next.getReference();
			if (!right_node_next.next.isMarked())
				if (right_node.next.compareAndSet(right_node_next, right_node_next, false, true))
					break;
		}while(true);
		
		if (!left_node.next.compareAndSet(right_node, right_node_next, false, false))
			search(right_node.value);
			
		return true;
	}

	@Override
	public boolean contains(T value) {
		Pair<Node> neighbours = search(value);

		if (neighbours.second != tail && neighbours.second.value.compareTo(value) == 0)
			return true;
		return false;
	}

	@Override
	public boolean isEmpty() {
		Node next;
		while(head.next.getReference() != tail) {
			next = head.next.getReference();
			if (!next.isMarked())
				return false;
			head.next.compareAndSet(next, next.next.getReference(), false, false);
	}

	Node head;
	Node tail;
}
