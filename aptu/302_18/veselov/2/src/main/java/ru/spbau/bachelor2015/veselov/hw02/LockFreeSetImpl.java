package ru.spbau.bachelor2015.veselov.hw02;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Optional;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final @NotNull HeadNode head = new HeadNode(null);

    public LockFreeSetImpl() { }

    @Override
    public boolean add(final @NotNull T value) {
        AbstractNode currentNode = head;

        while (true) {
            currentNode = findNodeBefore(currentNode, value);

            ListNode nextNode = currentNode.getNextNode();
            if (nextNode != null) {
                int comparisonResult = value.compareTo(nextNode.getData());
                if (comparisonResult == 0) {
                    return false;
                }

                if (comparisonResult > 0) {
                    continue;
                }
            }

            if (currentNode.insertDataAfterIfNext(value, nextNode)) {
                return true;
            }

            if (currentNode.isLogicallyDeleted()) {
                currentNode = head;
            }
        }
    }

    @Override
    public boolean remove(final @NotNull T value) {
        return findNodeWithValue(value).map(ListNode::deleteLogically).orElse(false);
    }

    @Override
    public boolean contains(final @NotNull T value) {
        return findNodeWithValue(value).isPresent();
    }

    @Override
    public boolean isEmpty() {
        return head.getNextNode() == null;
    }

    private @NotNull AbstractNode findNodeBefore(
        final @NotNull AbstractNode startNode,
        final @NotNull T value
    ) {
        AbstractNode currentNode = startNode;

        while (true) {
            ListNode nextNode = currentNode.getNextNode();
            if (nextNode == null || value.compareTo(nextNode.getData()) <= 0) {
                return currentNode;
            }

            currentNode = nextNode;
        }
    }

    private @NotNull Optional<ListNode> findNodeWithValue(final @NotNull T value) {
        AbstractNode currentNode = head;

        while (true) {
            ListNode nextNode = currentNode.getNextNode();
            if (nextNode == null) {
                return Optional.empty();
            }

            int comparisonResult = value.compareTo(nextNode.getData());
            if (comparisonResult == 0) {
                return Optional.of(nextNode);
            }

            if (comparisonResult < 0) {
                return Optional.empty();
            }

            currentNode = nextNode;
        }
    }

    private abstract class AbstractNode {
        protected final @NotNull AtomicMarkableReference<ListNode> referenceToNextNode;

        public AbstractNode(final @Nullable ListNode nextNode) {
            referenceToNextNode = new AtomicMarkableReference<>(nextNode, false);
        }

        public boolean insertDataAfterIfNext(
            final @NotNull T data,
            final @Nullable ListNode expectedNextNode
        ) {
            ListNode nodeToInsert = new ListNode(data, expectedNextNode);

            return referenceToNextNode.compareAndSet(
                expectedNextNode, nodeToInsert, false, false
            );
        }

        public @Nullable ListNode getNextNode() {
            while (true) {
                ListNode nextNode = referenceToNextNode.getReference();
                if (nextNode == null) {
                    return null;
                }

                boolean mark = referenceToNextNode.isMarked();

                if (nextNode.isLogicallyDeleted()) {
                    referenceToNextNode.compareAndSet(
                        nextNode,
                        nextNode.referenceToNextNode.getReference(),
                        mark,
                        mark
                    );
                } else {
                    return nextNode;
                }
            }
        }

        public abstract boolean isLogicallyDeleted();
    }

    private class HeadNode extends AbstractNode {
        public HeadNode(final @Nullable ListNode nextNode) {
            super(nextNode);
        }

        public boolean isLogicallyDeleted() {
            return false;
        }
    }

    private class ListNode extends AbstractNode {
        private final @NotNull T data;

        public ListNode(final @NotNull T data, final @Nullable ListNode nextNode) {
            super(nextNode);
            this.data = data;
        }

        public @NotNull T getData() {
            return data;
        }

        public boolean deleteLogically() {
            ListNode reference = referenceToNextNode.getReference();

            while (!referenceToNextNode.compareAndSet(
                reference,
                reference,
                false,
                true
            )) {
                if (referenceToNextNode.isMarked()) {
                    return false;
                }

                reference = referenceToNextNode.getReference();
            }

            return true;
        }

        public boolean isLogicallyDeleted() {
            return referenceToNextNode.isMarked();
        }
    }
}
