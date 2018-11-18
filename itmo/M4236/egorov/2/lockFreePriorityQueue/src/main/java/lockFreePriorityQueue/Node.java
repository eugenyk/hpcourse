package lockFreePriorityQueue;

import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class Node<E> {
	E data;
	AtomicReference<Node<E>> next;
	
	Node(E e) {
		data = e;
		next = new AtomicReference<>(null);
	}

	Node(E e, Node<E> next2) {
		data = e;
		next = new AtomicReference<Node<E>>(next2);
	}
}
