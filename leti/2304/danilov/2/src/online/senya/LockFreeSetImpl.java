package online.senya;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

public final class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    @Override
    public boolean add(@NotNull T value) {
        while(true) {
            Pair<Node, Node> foundNodes = this.findNodes(value);
            Node prev = foundNodes.first;
            Node curr = foundNodes.second;
            if (curr != null && Objects.equals(curr.getValue(), value)) {
                return false;
            }

            Node node = new Node(value, new AtomicMarkableReference<>(curr, false));
            if (Objects.equals(prev,null)) {
                if (this.head.compareAndSet(curr, node)) {
                    return true;
                }
            } else {
                boolean isCompareAndSetPassed = prev.getNextIsRemoved().compareAndSet(curr, node, false, false);
                if (isCompareAndSetPassed) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        Node predecessor;
        Node current;
        boolean casSuccess;
        Node successor;
        do {
            Pair<Node, Node> foundNodes = this.findNodes(value);
            predecessor = foundNodes.first;
            current = foundNodes.second;
            if (current == null || !Objects.equals(current.getValue(), value)) {
                return false;
            }

            successor = current.getNextIsRemoved().getReference();
            casSuccess = current.getNextIsRemoved().compareAndSet(successor, successor, false, true);
        } while(!casSuccess);

        if (Objects.equals(predecessor,null)) {
            this.head.compareAndSet(current, successor);
        } else {
            predecessor.getNextIsRemoved().compareAndSet(current, successor, false, false);
        }

        return true;
    }

    @Override
    public boolean contains(@NotNull T value) {
        Node curr;

        //noinspection StatementWithEmptyBody
        for(curr = this.head.get(); curr != null && curr.getValue().compareTo(value) < 0; curr = curr.getNextIsRemoved().getReference()) {
            // you spin me right round baby right round
            // оптимизатор плиз не оптимизируй
        }

        return curr != null && Objects.equals(curr.getValue(), value) && !curr.getNextIsRemoved().isMarked();
    }

    @Override
    public boolean isEmpty() {
        return this.head.get() == null;
    }

    @NotNull
    private Pair<Node, Node> findNodes(T value) {
        while(true) {
            Node pred = head.get();
            Node predNext;

            if (pred != null) {
                predNext = pred.getNextIsRemoved().getReference();
            } else {
                return new Pair<>(null, null);
            }

            if (pred.getNextIsRemoved().isMarked()) {
                head.compareAndSet(pred, pred.getNextIsRemoved().getReference());
                continue;
            }

            if (pred.getValue().compareTo(value) >= 0) {
                return new Pair<>(null, pred);
            }


            Node curr = predNext;
            boolean retry = false;
            while(curr != null) {
                Node succ = curr.getNextIsRemoved().getReference();
                boolean isCurrDeleted = curr.getNextIsRemoved().isMarked();
                if (isCurrDeleted) {
                    boolean isCompareAndSetPassed = pred.getNextIsRemoved().compareAndSet(curr, succ, false, false);
                    if (!isCompareAndSetPassed) {
                        retry = true;
                        break;
                    }

                    curr = succ;
                } else {
                    if (curr.getValue().compareTo(value) >= 0) {
                        return new Pair<>(pred, curr);
                    }

                    pred = curr;
                    curr = succ;
                }
            }

            if (retry) {
                continue;
            }

            return new Pair<>(pred, null);

        }
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();

        for(Node curr = this.head.get(); curr != null; curr = curr.getNextIsRemoved().getReference()) {
            builder.append(curr.getValue()).append("->");
        }

        return builder.toString();
    }

    private final class Node {
        @NotNull
        private final T value;
        @NotNull
        private final AtomicMarkableReference<Node> nextIsRemoved;

        @NotNull
        final T getValue() {
            return this.value;
        }

        @NotNull
        final AtomicMarkableReference<Node> getNextIsRemoved() {
            return this.nextIsRemoved;
        }

        Node(@NotNull final T value, @NotNull final AtomicMarkableReference<Node> nextIsRemoved) {
            this.value = value;
            this.nextIsRemoved = nextIsRemoved;
        }
    }

    private final class Pair<A, B> {
        // избави нас от premature publication
        final A first;
        final B second;

        Pair(final A first, final B second) {
            this.first = first;
            this.second = second;
        }

    }

}
