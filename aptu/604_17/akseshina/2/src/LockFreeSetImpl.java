
import java.util.concurrent.atomic.AtomicMarkableReference;


public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final class Node {
        private final T value;
        // an item is in the set if, and only if it is in an unmarked reachable node
        private AtomicMarkableReference<Node> next;

        Node(T value) {
            this.value = value;
            next = new AtomicMarkableReference<>(null, false);
        }

        private Node(final T value, Node next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        private int compareTo(T value2) {
            if (value == null || value2 == null) {
                return -1;
            }
            return value.compareTo(value2);
        }
    }

    private Node tail;
    private Node head;

    public LockFreeSetImpl() {
        tail = new Node(null);
        head = new Node(null, tail);
    }

    class Window {
        private Node prev, curr;
        Window(Node myPred, Node myCurr) {
            prev = myPred; curr = myCurr;
        }
    }

    //takes a head node and a key a, and traverses
    //the list, seeking to set prev to the node with the largest key less than a, and curr
    //to the node with the least key greater than or equal to a
    private Window find(Node head, T value) {
        Node prevNode = null;
        Node currNode = null;
        Node nextNode = null;
        boolean[] marked = {false};
        boolean removeSucess;

        main_loop: while (true) {
            prevNode = head;
            currNode = prevNode.next.getReference();

            while (true) {
                nextNode = currNode.next.get(marked);

                while (marked[0]) {
                    // attempt to physically remove the node
                    removeSucess = prevNode.next.compareAndSet(currNode, nextNode,false, false);
                    // fails if either value or ref has changed
                    if (!removeSucess)
                        continue main_loop;
                    currNode = nextNode;
                    nextNode = currNode.next.get(marked);
                }

                if (currNode.compareTo(value) >= 0 || currNode == tail)
                    return new Window(prevNode, currNode);
                prevNode = currNode;
                currNode = nextNode;
            }

        }
    }

    @Override
    public boolean add(T value) {
        while (true) {

            Window window = find(head, value);
            Node prevNode = window.prev;
            Node currNode = window.curr;

            if (currNode.value == value) {
                return false;
            } else {
                Node newNode = new Node(value, null);
                newNode.next = new AtomicMarkableReference(currNode, false);
                if (prevNode.next.compareAndSet(currNode, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        boolean removeSucess;
        while (true) {
            Window window = find(head, value);
            Node prevNode = window.prev;
            Node currNode = window.curr;

            if (currNode.value != value) {
                return false;
            } else {
                Node nextNode = currNode.next.getReference();

                // attempt to physically remove the node
                removeSucess = currNode.next.compareAndSet(nextNode, nextNode, false, true);
                // fails if either value or ref has changed
                if (!removeSucess)
                    continue;
                prevNode.next.compareAndSet(currNode, nextNode, false, false);

                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        boolean[] marked = {false};
        Node currNode = head;
        while (currNode.compareTo(value) < 0 && currNode != tail) {
            currNode = currNode.next.getReference();
            Node nextNode = currNode.next.get(marked);
        }
        return (currNode.value == value && !marked[0]);
    }

    @Override
    public boolean isEmpty() {
        Node prevNode = null;
        Node currNode = null;
        Node nextNode = null;
        boolean[] marked = {false};
        boolean removeSucess;

        main_loop:
        while (true) {
            prevNode = head;
            currNode = prevNode.next.getReference();

            nextNode = currNode.next.get(marked);
            while (marked[0]) {
                // attempt to physically remove the node
                removeSucess = prevNode.next.compareAndSet(currNode, nextNode, false, false);
                // fails if either value or ref has changed
                if (!removeSucess)
                    continue main_loop;
                currNode = nextNode;
                nextNode = currNode.next.get(marked);
            }

            return currNode == tail;

        }
    }

}
