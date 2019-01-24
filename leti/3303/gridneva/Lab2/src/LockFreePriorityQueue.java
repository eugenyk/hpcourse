package lab2;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;


public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

	
		private final Node<E> tail = new Node<>(null, null);
		private final Node<E> head = new Node<>(null, tail);


		@Override
		public boolean offer(E e) {
			while (true) 
			{
				PositionNodes<E> position = search(e);
				Node<E> previous = position.getLeftNode();
				Node<E> next = position.getRightNode();
				Node<E> newNode = new Node<>(e, next);
				if(previous.compareAndSetNext(next, newNode)) 
				{return true;}	
			}
		}
		
		@Override
		public E peek() {
			
			Node<E> first = null;
			Node<E> current = null;
			Node<E> next = null;
	        Node<E> last = null;
			boolean[] markHolder = {false};
			while(true) {
				first = head;
				current = first.next.getReference();
				next = current.next.get(markHolder);
				last = tail;
				if (markHolder[0]) {
					first.next.compareAndSet(current, next, false, false);
				}
				else if (current != last) {return current.getValue();}
				else {
					return null;
				}
			}
		}

		@Override
		public E poll() {
			
			Node<E> first = null;
			Node<E> current = null;
			Node<E> next = null;
	        Node<E> last = null;
			boolean[] markHolder = {false};
			while(true) {
				first = head;
				current = first.next.getReference();
				next = current.next.get(markHolder);
				last = tail;
				if (markHolder[0]) {
					first.next.compareAndSet(current, next, false, false);
				}
				else if (current != last) {
					if (current.next.compareAndSet(next, next, false, true)) {
						first.next.compareAndSet(current, next, false, false);
						return current.getValue();
					}
				}
				else {
					return null;
				}
				
			}
		}
		
		@Override
		public Iterator<E> iterator() 
		{
			return null;
		}

		@Override
		public int size() 
		{
			Node<E> current = head.getNext();
			int i = 0;
			while (current != tail){
				if (!current.isMarked()){
					i++;
				}
				current = current.getNext();
			}
			return i;
		}

		@Override
		public boolean isEmpty() {
			return size() == 0;
		}

		   
	   
	private static class Node<E extends Comparable<E>> {
		private final E data;
		private AtomicMarkableReference<Node<E>> next;

		Node(E value, Node<E> next) {
			this.data = value;
			this.next = new AtomicMarkableReference<>(next, false);
		}

		E getValue() {
			return data;
		}
				
		Node<E> getNext() {
			return next.getReference();
		}
				
		Node<E> getNext(boolean markHolder[]) {
			return next.get(markHolder);
		}
				
		boolean isMarked() {
			return next.isMarked();
		}

		boolean compareAndSetNext(Node<E> nextOld, Node<E> nextNew) {
			return next.compareAndSet(nextOld, nextNew, false, false);
		}
				
	}
	
	private static class PositionNodes<E extends Comparable<E>> {
	       private final Node<E> left;
	       private final Node<E> right;

	       PositionNodes(Node<E> leftNode, Node<E> rightNode) {
	           left = leftNode;
	           right = rightNode;
	       }

	       Node<E> getLeftNode() {
	           return left;
	       }

	       Node<E> getRightNode() {
	           return right;
	       }
	   }
	
	private PositionNodes<E> search(E e) {
		Node<E> rightNode;
		Node<E> leftNodeNext = null;
		Node<E> leftNode = null;
		do {
		       Node<E> t = head;
		       do {
		             if (!t.isMarked()) {
		                 leftNode = t;
		                 leftNodeNext = t.getNext();
		               }
		               t = t.getNext();
		               if (t == tail)
		                   break;
		          } while (t.isMarked() || (e != null && t.getValue().compareTo(e) <= 0));

		         rightNode = t;
		         if (leftNodeNext == rightNode) {
		             if ((rightNode != tail) && rightNode.next.isMarked()) {
		                 continue;
		               } 
		             else {
		                   return new PositionNodes<>(leftNode, rightNode); 
		              }
		           }

		          if (leftNode.compareAndSetNext(leftNodeNext, rightNode)) {
		               if (rightNode == tail || !rightNode.next.isMarked()) {
		                   return new PositionNodes<>(leftNode, rightNode);
		               }
		           }
		       } while (true);

		}
	

	   private Node<E> firstNode() {
	       Node<E> left = head;
	       boolean[] marklHolder = {false};
	       while (true) {
	           Node<E> current = left.getNext();
	           Node<E> next = current.getNext(marklHolder);
	           if(marklHolder[0]) {
	               left.compareAndSetNext(current, next);
	               continue;
	           }
	           return current;
	       }
	   }

	
}