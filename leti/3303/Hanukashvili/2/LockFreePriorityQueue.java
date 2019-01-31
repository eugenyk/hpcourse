package lab2;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;


public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

	private static class Node<E extends Comparable<E>> {
		private final E value;
		private AtomicMarkableReference<Node<E>> next;

		Node(E value, Node<E> next) {
			this.value = value;
			this.next = new AtomicMarkableReference<>(next, false);
		}

		E getValue() {
			return value;
		}
		boolean isMarked() {
			return next.isMarked();
		}

		boolean compareAndSetNext(Node<E> nextOld, Node<E> nextNew) {
			return next.compareAndSet(nextOld, nextNew, false, false);
		}

		Node<E> getNext() {
			return next.getReference();
		}

		Node<E> getNext(boolean flagDeletion[]) {
			return next.get(flagDeletion);
		}

		boolean nextIsMarked() {
			return next.isMarked();
		}
	}
		private final Node<E> tail = new Node<>(null, null);
		private final Node<E> head = new Node<>(null, tail);


		@Override
		public Iterator<E> iterator() {
			return null;
		}

		@Override
		public int size() {
			Node<E> currenct = head.getNext();
			int i = 0;
			while (currenct != tail){
				if (!currenct.nextIsMarked()){
					i++;
				}
				currenct = currenct.getNext();
			}
			return i;
		}

		@Override
		public boolean offer(E e) {

			while (true) {
				Position<E> pair = findPosition(e);
				Node<E> previous = pair.getLeft();
				Node<E> next = pair.getRight();
				Node<E> newNode = new Node<>(e, next);
				if(previous.compareAndSetNext(next, newNode)) {
					return true;
				}	
			}
		}

		@Override
		public E poll() {
			while (true) {
				Node<E> position = search();
				if(position == tail) {
					return null;
				}
				Node<E> newNext = position.getNext();
				if(position.next.compareAndSet(newNext, newNext, false, true)) {
					head.compareAndSetNext(position, newNext);
					return position.getValue();
				}
			}
		}

		@Override
		public E peek() {
			Node<E> position = search();
			if (position == tail){
				return null;
			}
			return position.getValue();
		}


		@Override
		public boolean isEmpty() {
			return search().getValue() == null;
		}

		   private Position<E> findPosition(E e) {
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
		               } else {
		                   return new Position<>(leftNode, rightNode); 
		               }
		           }

		           if (leftNode.compareAndSetNext(leftNodeNext, rightNode)) {
		               if (rightNode == tail || !rightNode.next.isMarked()) {
		                   return new Position<>(leftNode, rightNode);
		               }
		           }
		       } while (true);

		}

		   private Node<E> search() {
		       Node<E> left = head;
		       boolean[] flagDeletion = {false};
		       while (true) {
		           Node<E> current = left.getNext();
		           Node<E> next = current.getNext(flagDeletion);
		           if(flagDeletion[0]) {
		               left.compareAndSetNext(current, next);
		               continue;
		           }
		           return current;
		       }
		   }



		   private static class Position<E extends Comparable<E>> {
		       private final Node<E> left;
		       private final Node<E> right;

		       Position(Node<E> leftNode, Node<E> rightNode) {
		           left = leftNode;
		           right = rightNode;
		       }

		       Node<E> getLeft() {
		           return left;
		       }

		       Node<E> getRight() {
		           return right;
		       }
		   }
}
