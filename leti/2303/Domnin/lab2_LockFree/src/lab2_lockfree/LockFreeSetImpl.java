/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package lab2_lockfree;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

	private final Node<T> head;

	public LockFreeSetImpl() {
		head = new Node<>(null);
	}

        
        @Override
	public boolean add(T value) {
		Node<T> newNode = new Node<>(value);
		while (true) {
			TwoNodes<T> nodes = searchPositionByValue(value);
			if (value.equals(nodes.prev.value) && !nodes.prev.next.isMarked()) 
                            return false;
			newNode.next = new AtomicMarkableReference<>(nodes.curr, false);
			if (nodes.prev.next.compareAndSet(nodes.curr, newNode, false, false)) 
                            return true;	
		}
	}

	@Override
	public boolean remove(T value) {
		while (true) {
			TwoNodes<T> nodes = searchPositionByValue(value);
			if (!value.equals(nodes.prev.value)) 
                            return false;	
			Node<T> next = nodes.prev.next.getReference();
			if (nodes.prev.next.compareAndSet(next, next, false, true))
                            return true;
		}
	}

        @Override
	public boolean contains(T value) {
                TwoNodes<T> nodes = searchPositionByValue(value);              
                return (value.equals(nodes.prev.value) && !nodes.prev.next.isMarked());  
	}
        
        @Override
	public boolean isEmpty() {
            while(true) {
                Node<T> node = head.next.getReference();
                if (node == null)
                    return true;
                if (node.next.isMarked()) {
                    head.next.compareAndSet(node, node.next.getReference(), false, false);
                    continue;
                    }
                return false;
            }
	}

	
        private TwoNodes<T> searchPositionByValue(T value) {
		while (true) {
			Node<T> prev = head;
			Node<T> curr = head.next.getReference();

			while (true) {
				if (curr == null)
					return new TwoNodes<>(prev, null);
                                if (curr.value.compareTo(value) > 0) 
					return new TwoNodes<>(prev, curr);
				if (curr.next.isMarked()) { 
                                    if (prev.next.compareAndSet(curr, curr.next.getReference(), false, false)) {
                                            curr = prev.next.getReference();
                                            continue;
                                    }
                                    break;
				}
				prev = curr;
				curr = curr.next.getReference();
			}
		}
	}
        
         
        private class Node<T> {
		T value;
		AtomicMarkableReference<Node<T>> next;

		Node(T value) {
			this.value = value;
			next = new AtomicMarkableReference<>(null, false);
		}
	}

	private class TwoNodes<T> {
		Node<T> prev;
		Node<T> curr;

		TwoNodes(Node<T> prev, Node<T> curr) {
			this.prev = prev;
			this.curr = curr;
		}
	}
	
}
