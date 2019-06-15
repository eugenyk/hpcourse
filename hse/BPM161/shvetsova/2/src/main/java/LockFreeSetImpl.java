import com.sun.istack.internal.NotNull;
import com.sun.istack.internal.Nullable;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T> implements LockFreeSetInterface<T> {
    private static class Node<T> {
        private volatile AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);
        private final T value;

        Node(final T value) {
            this.value = value;
        }

        @Nullable
        AtomicMarkableReference<Node<T>> getNext() {
            return next;
        }

        T getValue() {
            return value;
        }
    }

    private AtomicMarkableReference<Node<T>> list = new AtomicMarkableReference<>(new Node<>(null), false);

    public boolean add(@NotNull T value) {
        AtomicMarkableReference<Node<T>> currentNode;
        Node<T> newNode = new Node<>(value);
        while (true) {
             currentNode = list;
             while (((!value.equals(currentNode.getReference().getValue()) || currentNode.isMarked())
                     && currentNode.getReference().getNext() != null)) {
                currentNode = currentNode.getReference().getNext();
             }
             if (value.equals(currentNode.getReference().getValue()) && !currentNode.isMarked()) {
                 return false;
             }
             if (currentNode.getReference().getNext().compareAndSet(null, newNode, false, false)) {
                 return true;
             }
        }
    }


    public boolean remove(@NotNull T value) {
        AtomicMarkableReference<Node<T>> currentNode;
        while (true) {
            currentNode = list;
            while (currentNode != null
                    && ((!value.equals(currentNode.getReference().getValue()) || currentNode.isMarked()))) {
                currentNode = currentNode.getReference().getNext();
            }
            if (currentNode == null) {
                return false;
            }

            Node<T> nextNode = currentNode.getReference();
            if (value.equals(currentNode.getReference().getValue())) {
                if (currentNode.compareAndSet(nextNode, nextNode, false, true)) {
                    return true;
                }
            }
        }
    }


    public boolean contains(T value) {
        AtomicMarkableReference<Node<T>> currentNode;
        while (true) {
            currentNode = list;
            while (currentNode != null
                    && ((!value.equals(currentNode.getReference().getValue()) || currentNode.isMarked()))) {
                currentNode = currentNode.getReference().getNext();
            }
            if (currentNode == null) {
                return false;
            }
            if (value.equals(currentNode.getReference().getValue()) && !currentNode.isMarked()) {
                return true;
            }
        }
    }

    public boolean isEmpty() {
        AtomicMarkableReference<Node<T>> currentNode;

        currentNode = list;
        while (currentNode != null && currentNode.isMarked()) {
            currentNode = currentNode.getReference().getNext();
        }
        return currentNode == null;
    }

    private List<T> getSnapshot() {
        List<T> oldValues = null;
        while (true) {
            List<T> newValues = new ArrayList<>();
            AtomicMarkableReference<Node<T>> currentNode = list;
            while (currentNode != null) {
                if (!currentNode.isMarked()) {
                    newValues.add(currentNode.getReference().getValue());
                }
                currentNode = currentNode.getReference().getNext();
            }
            if (oldValues != null) {
                if (oldValues.size() != newValues.size()) continue;

                boolean equals = true;
                for (int i = 0; i < oldValues.size(); i++) {
                    if (!oldValues.get(i).equals(newValues.get(i))) {
                        equals = false;
                        break;
                    }
                }
                if (equals) {
                    break;
                }
            }
            oldValues = newValues;
        }
        return oldValues;
    }

    public java.util.Iterator<T> iterator() {
        return getSnapshot().iterator();
    }
}