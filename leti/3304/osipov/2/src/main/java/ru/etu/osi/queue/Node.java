package ru.etu.osi.queue;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<E extends Comparable<E>> {
	public E data = null;
	public AtomicMarkableReference<Node<E>> next = null;
	
	public Node(E data, Node<E> next)
	{
		this.data = data;
		this.next = new AtomicMarkableReference<Node<E>>(next, false);
	}
	
	public Node(E data)
	{
		this.data = data;
		this.next = new AtomicMarkableReference<Node<E>>(null, false);
	}
}
