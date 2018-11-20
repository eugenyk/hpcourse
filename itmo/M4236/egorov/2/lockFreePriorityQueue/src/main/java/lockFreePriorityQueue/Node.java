package lockFreePriorityQueue;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<E> {
	E data;
	AtomicMarkableReference<Node<E>> next;
	
	Node(E e) {
		data = e;
		next = new AtomicMarkableReference<>(null, true);
	}

	Node(E e, Node<E> next2) {
		data = e;
		next = new AtomicMarkableReference<Node<E>>(next2, true);
	}
}
