package LockFreePriorityQueue;

import java.util.Queue;

public interface PriorityQueue<E extends Comparable<E>> extends Queue<E> {
	boolean isEmpty();

}
