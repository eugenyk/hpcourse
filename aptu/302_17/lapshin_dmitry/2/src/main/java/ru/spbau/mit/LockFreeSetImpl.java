package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final AtomicReference<InternalNode> root = new AtomicReference<>();
    private final AtomicInteger count = new AtomicInteger(0);

    public LockFreeSetImpl() {
        root.set(new InternalNode(ValueType.POSINF, new LeafNode(ValueType.NEGINF), new LeafNode(ValueType.POSINF)));
    }

    private class SearchResult {
        InternalNode gp;
        InternalNode p;
        LeafNode node;
        State gpState;
        State pState;

        private SearchResult(InternalNode gp, InternalNode p, LeafNode node, State gpState, State pState) {
            this.gp = gp;
            this.p = p;
            this.node = node;
            this.gpState = gpState;
            this.pState = pState;
        }
    }

    private SearchResult search(T key) {
        InternalNode gp = null;
        InternalNode p = null;
        Node node = root.get();
        State gpState = null;
        State pState = null;

        while (InternalNode.class.isInstance(node)) {
            InternalNode nodeInternal = (InternalNode) node;
            gp = p;
            gpState = pState;
            p = nodeInternal;
            pState = p.getState();
            if (node.compareWithKey(key) < 0) {
                node = nodeInternal.getLeft();
            } else {
                node = nodeInternal.getRight();
            }
        }

        return new SearchResult(gp, p, (LeafNode) node, gpState, pState);
    }

    @Override
    public boolean add(T value) {
        if (value == null)
            throw new NullPointerException();
        while (true) {
            SearchResult searchResult = search(value);
            if (value.equals(searchResult.node.getKey()))
                return false;
            if (searchResult.pState != State.CLEAN) {
                continue;
            }

            LeafNode newLeaf = new LeafNode(value);
            LeafNode newSibling = new LeafNode(searchResult.node.getValue());
            InternalNode newInternal;
            if (newSibling.compareTo(newLeaf) < 0) {
                newInternal = new InternalNode(newLeaf.getKey(), newSibling, newLeaf);
            } else {
                newInternal = new InternalNode(newSibling.getKey(), newLeaf, newSibling);
            }

            if (searchResult.p.compareAndSetState(searchResult.pState, State.IFLAG)) {
                searchResult.p.setChild(newInternal);
                searchResult.p.setState(State.CLEAN);
                count.getAndIncrement();
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        if (value == null)
            throw new NullPointerException();
        while (true) {
            SearchResult searchResult = search(value);
            if (!value.equals(searchResult.node.getKey()))
                return false;
            if (searchResult.pState != State.CLEAN) {
                continue;
            }
            if (searchResult.gpState != State.CLEAN) {
                continue;
            }

            if (searchResult.gp.compareAndSetState(searchResult.gpState, State.DFLAG)) {
                boolean markSucceeded = searchResult.p.compareAndSetState(searchResult.pState, State.MARKED);
                if (markSucceeded) {
                    Node newChild;
                    if (searchResult.p.getRight() == searchResult.node)
                        newChild = searchResult.p.getLeft();
                    else
                        newChild = searchResult.p.getRight();
                    searchResult.gp.setChild(newChild);
                    markSucceeded = true;
                }
                searchResult.gp.setState(State.CLEAN);
                if (markSucceeded) {
                    count.getAndDecrement();
                    return true;
                }
            }
        }
    }

    @Override
    public boolean contains(T value) {
        if (value == null)
            throw new NullPointerException();
        SearchResult searchResult = search(value);
        return value.equals(searchResult.node.getKey());
    }

    @Override
    public boolean isEmpty() {
        return count.get() == 0;
    }

    private enum State {
        CLEAN,
        IFLAG,
        DFLAG,
        MARKED
    }

    enum ValueType {
        NORMAL,
        POSINF,
        NEGINF
    }

    private final class Value {
        private final ValueType type;
        private final T value;

        private Value(ValueType type, T value) {
            if ((type == ValueType.NORMAL) == (value == null))
                throw new IllegalArgumentException();
            this.type = type;
            this.value = value;
        }
    }

    private abstract class Node implements Comparable<Node> {
        private final Value value;

        private Node(Value value) {
            this.value = value;
        }

        T getKey() {
            return value.value;
        }

        int compareWithKey(T key) {
            switch (value.type) {
                case NORMAL:
                    return key.compareTo(value.value);
                case POSINF:
                    return -1;
                case NEGINF:
                    return 1;
            }
            return 0;
        }

        @Override
        public int compareTo(Node o) {
            switch (value.type) {
                case NORMAL:
                    return o.compareWithKey(value.value);
                case POSINF:
                    return (o.value.type != ValueType.POSINF) ? 1 : 0;
                case NEGINF:
                    return (o.value.type != ValueType.NEGINF) ? -1 : 0;
            }
            return 0;
        }

        Value getValue() {
            return value;
        }
    }

    private final class InternalNode extends Node {
        private final AtomicReference<Node> left = new AtomicReference<>();
        private final AtomicReference<Node> right = new AtomicReference<>();
        private final AtomicReference<State> state = new AtomicReference<>(State.CLEAN);

        private InternalNode(T key, Node left, Node right) {
            this(new Value(ValueType.NORMAL, key), left, right);
        }

        private InternalNode(ValueType type, Node left, Node right) {
            this(new Value(type, null), left, right);
        }

        private InternalNode(Value value, Node left, Node right) {
            super(value);
            this.left.set(left);
            this.right.set(right);
        }

        private State getState() {
            return state.get();
        }

        private void setState(State newState) {
            state.set(newState);
        }

        private boolean compareAndSetState(State expectedState, State newState) {
            return state.compareAndSet(expectedState, newState);
        }

        private Node getLeft() {
            return left.get();
        }

        private Node getRight() {
            return right.get();
        }

        private void setChild(Node newChild) {
            if (newChild.compareTo(this) < 0) {
                left.set(newChild);
            } else {
                right.set(newChild);
            }
        }
    }

    private final class LeafNode extends Node {
        private LeafNode(T key) {
            this(new Value(ValueType.NORMAL, key));
        }

        private LeafNode(ValueType type) {
            this(new Value(type, null));
        }

        private LeafNode(Value value) {
            super(value);
        }
    }

}

