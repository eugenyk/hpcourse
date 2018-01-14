import com.sun.istack.internal.NotNull;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Created by xd720p on 29.10.2017.
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    private class Node {
        @NotNull
        private final T value;

        //atomicmarkablereference to work using CAS
        @NotNull
        private final AtomicMarkableReference<Node> nextNode;

        public Node(@NotNull T value, @NotNull AtomicMarkableReference<Node> nextNode) {
            this.value = value;
            this.nextNode = nextNode;
        }

        T getValue() {
            return value;
        }

        AtomicMarkableReference<Node> getNextNode() {
            return nextNode;
        }
    }

    private class NodeWindow {
        private final Node prev;
        private final Node prevNext;

        NodeWindow(Node prev, Node prevNext) {
            this.prev = prev;
            this.prevNext = prevNext;
        }

        Node getPrev() {
            return prev;
        }

        Node getPrevNext() {
            return prevNext;
        }
    }

    @Override
    public boolean add(T value) {
        while(true) {
            NodeWindow nodeWindow = find(value);
            Node currNode = nodeWindow.getPrevNext();
            Node prevNode = nodeWindow.getPrev();

            if (Objects.equals(currNode.getValue(), value)) {
                return false;
            } else {
                Node insertNode = new Node(value, new AtomicMarkableReference<>(currNode, false));
                if (prevNode.getNextNode().compareAndSet(currNode, insertNode, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while(true) {
            NodeWindow nodeWindow = find(value);
            Node currNode = nodeWindow.getPrevNext();

            if (currNode.getValue().compareTo(value) != 0) {
                return false;
            } else {
                Node nextNode = currNode.getNextNode().getReference();
                if (currNode.getNextNode().compareAndSet(nextNode, nextNode, false, true)) {
                    return true;
                }
            }
        }
    }

    //move window throw set until value
    private NodeWindow find(@NotNull T value) {
        while(true) {
            Node prevNode = head.get();
            Node currNode = prevNode.getNextNode().getReference();

        retry: while(true) {
                boolean currentMarked = currNode.isMarked();
                Node nextNode = currNode.getNextNode().getReference();

                if (currentMarked) {
                    if (!prevNode.getNextNode().compareAndSet(currNode, nextNode, currentMarked, false)) {
                        continue retry;
                    } else {
                        currNode = nextNode;
                    }
                } else {
                    if (currNode.getValue().compareTo(value) >= 0) {
                        return new NodeWindow(prevNode, currNode);
                    } else {
                        prevNode = currNode;
                        currNode = nextNode;
                    }
                }
            }
        }
    }

    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean contains(T value) {
        if (value == null) return false;
        Node current;
        //walk until current value < searching value and current != null
        for (current = head.get(); current != null && current.getValue().compareTo(value) < 0; current = current.getNextNode().getReference()) {
        }

        return current != null && Objects.equals(value, current.getValue());
    }

    @Override
    public boolean isEmpty() {
        return this.head.get() == null;
    }
}