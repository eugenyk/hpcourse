package task;

import java.util.concurrent.atomic.AtomicStampedReference;
import java.util.logging.Logger;

public class ThreadSafeSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Logger logger = Logger.getLogger(getClass().getName());
    private final AtomicStampedReference<Node<T>> rootRef =
            new AtomicStampedReference<Node<T>>(null, Node.State.CLEAN.ordinal());

    private AtomicStampedReference<Node<T>> decision(Node<T> node, T value) {
        if (node.getValue().compareTo(value) > 0) {
            return node.getLeft();
        }

        return node.getRight();
    }

    public boolean add(T value) {
        /*
         * 3 cases:
         * - no nodes in the tree
         * - a node in the tree (it's a leaf)
         * - more than 1
         */

        while (true) {
            tryAgain:
            {
                final Node<T> newLeaf = new Node<T>(value);
                final Node<T> root = rootRef.getReference();

                if (root == null) {
                    if (!rootRef.compareAndSet(null, newLeaf, Node.State.CLEAN.ordinal(), Node.State.CLEAN.ordinal())) {
                        logger.info("can't insert into rootRef");
                        break tryAgain;
                    }

                    return true;
                } else if (root.isLeaf()) {
                    if (!rootRef.compareAndSet(root, root, Node.State.CLEAN.ordinal(), Node.State.ADD.ordinal())) {
                        logger.info("can't set ADD  state to rootRef-leaf");
                        break tryAgain;
                    }

                    /* somebody could add children */
                    if (!root.isLeaf()) {
                        if (!rootRef.compareAndSet(root, root, Node.State.ADD.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't back rootRef");
                        }
                        break tryAgain;
                    }

                    if (root.getValue().compareTo(value) != 0) {
                        Node<T> parent = root.getValue().compareTo(value) > 0
                                ? new Node<T>(root.getValue(), newLeaf, root)
                                : new Node<T>(value, root, newLeaf);

                        if (!rootRef.compareAndSet(root, parent, Node.State.ADD.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't change rootRef");
                        }

                        return true;
                    } else {
                        if (!rootRef.compareAndSet(root, root, Node.State.ADD.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't return rootRef");
                        }

                        return false;
                    }
                } else {
                    AtomicStampedReference<Node<T>> prevRef = rootRef;
                    Node<T> prev;
                    AtomicStampedReference<Node<T>> curRef;
                    Node<T> cur;

                    while (true) {
                        if (prevRef.getStamp() != Node.State.CLEAN.ordinal()) {
                            break tryAgain;
                        }
                        prev = prevRef.getReference();
                        curRef = decision(prev, value);
                        cur = curRef.getReference();

                        if (cur.isLeaf()) {
                            break;
                        }

                        prevRef = curRef;
                    }

                    if (!prevRef.compareAndSet(prev, prev, Node.State.CLEAN.ordinal(), Node.State.ADD.ordinal())) {
                        logger.info("can't turn next node in add");
                        break tryAgain;
                    }

                    cur = curRef.getReference();
                    if (cur == null || !cur.isLeaf()) {
                        if (!prevRef.compareAndSet(prev, prev, Node.State.ADD.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't back in CLEAN state");
                        }
                        break tryAgain;
                    }

                    /* node is a leaf, we should duplicate it as new parent node */
                    cur = curRef.getReference();

                    boolean added = false;
                    if (cur.getValue().compareTo(value) != 0) {
                        Node<T> parent = cur.getValue().compareTo(value) > 0
                                ? new Node<T>(cur.getValue(), newLeaf, cur)
                                : new Node<T>(value, cur, newLeaf);

                        if (prev.getValue().compareTo(value) > 0) {
                            if (!prev.getLeft().compareAndSet(cur, parent, Node.State.CLEAN.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't CAS left child");
                            }
                        } else {
                            if (!prev.getRight().compareAndSet(cur, parent, Node.State.CLEAN.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't CAS right child");
                            }
                        }

                        added = true;
                    }

                    if (!prevRef.compareAndSet(prev, prev, Node.State.ADD.ordinal(), Node.State.CLEAN.ordinal())) {
                        throw new RuntimeException("impossible: can't back in CLEAN state");
                    }

                    return added;
                }
            }
        }
    }

    public boolean remove(T value) {
        /*
         * 3 cases:
         * - no nodes in the tree
         * - a node in the tree (it's a leaf)
         * - more than 1
         */

        while (true) {
            tryAgain: {
                final Node<T> root = rootRef.getReference();

                if (root == null) {
                    return false;
                } else if (root.isLeaf()) {
                    if (!rootRef.compareAndSet(root, root, Node.State.CLEAN.ordinal(), Node.State.REM1.ordinal())) {
                        logger.info("can't set REM1 state to rootRef-leaf");
                        break tryAgain;
                    }

                    if (!root.isLeaf()) {
                        if (!rootRef.compareAndSet(root, root, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't back rootRef-leaf");
                        }
                        break tryAgain;
                    }

                    if (root.getValue().compareTo(value) == 0) {
                        if (!rootRef.compareAndSet(root, null, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't remove rootRef-leaf");
                        }
                        return true;
                    } else {
                        if (!rootRef.compareAndSet(root, root, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't remove rootRef-leaf");
                        }
                        return false;
                    }
                } else {
                    AtomicStampedReference<Node<T>> rootLeafRef = decision(root, value);
                    Node<T> rootLeaf = rootLeafRef.getReference();

                    if (rootLeaf.isLeaf()) {
                        if (!rootRef.compareAndSet(root, root, Node.State.CLEAN.ordinal(), Node.State.REM1.ordinal())) {
                            logger.info("can't turn root in rem1");
                            break tryAgain;
                        }

                        if (!rootLeaf.isLeaf()) {
                            if (!rootRef.compareAndSet(root, root, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't CAS left child");
                            }
                            break tryAgain;
                        }

                        if (rootLeaf.getValue().compareTo(value) == 0) {
                            if (root.getValue().compareTo(value) > 0) {
                                if (!rootRef.compareAndSet(root, root.getRight().getReference(), Node.State.REM1.ordinal(), root.getRight().getStamp())) {
                                    throw new RuntimeException("impossible: can't CAS right child");
                                }
                            } else {
                                if (!rootRef.compareAndSet(root, root.getLeft().getReference(), Node.State.REM1.ordinal(), root.getRight().getStamp())) {
                                    throw new RuntimeException("impossible: can't CAS left child");
                                }
                            }

                            return true;
                        } else {
                            if (!rootRef.compareAndSet(root, root, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't back root in CLEAN state");
                            }
                            return false;
                        }
                    } else {
                        AtomicStampedReference<Node<T>> prevRef = rootRef;
                        Node<T> prev;
                        AtomicStampedReference<Node<T>> curRef;
                        Node<T> cur;
                        AtomicStampedReference<Node<T>> leafRef;
                        Node<T> leaf;

                        while (true) {
                            if (prevRef.getStamp() != Node.State.CLEAN.ordinal()) {
                                break tryAgain;
                            }

                            prev = prevRef.getReference();
                            curRef = decision(prev, value);

                            if (curRef.getStamp() != Node.State.CLEAN.ordinal()) {
                                break tryAgain;
                            }
                            cur = curRef.getReference();

                            if (cur == null) {
                                return false;
                            }

                            leafRef = decision(cur, value);
                            leaf = leafRef.getReference();

                            if (leaf == null) {
                                return false;
                            }

                            if (leaf.isLeaf()) {
                                break;
                            }

                            prevRef = curRef;
                        }

                        if (!prevRef.compareAndSet(prev, prev, Node.State.CLEAN.ordinal(), Node.State.REM1.ordinal())) {
                            logger.info("can't turn prev in rem1");
                            break tryAgain;
                        }
                        curRef = decision(prev, value);
                        cur = curRef.getReference();

                        if (!curRef.compareAndSet(cur, cur, Node.State.CLEAN.ordinal(), Node.State.REM2.ordinal())) {
                            logger.info("can't turn cur in rem2");
                            if (!prevRef.compareAndSet(prev, prev, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't back prev to clean");
                            }
                            break tryAgain;
                        }

                        if (cur == null || cur.isLeaf()) {
                            if (!curRef.compareAndSet(cur, cur, Node.State.REM2.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't back cur to clean");
                            }
                            if (!prevRef.compareAndSet(prev, prev, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't back prev to clean");
                            }
                            break tryAgain;
                        }


                        leafRef = decision(cur, value);
                        leaf = leafRef.getReference();

                        boolean removed = false;
                        if (leaf.getValue().compareTo(value) == 0) {
                            if (cur.getValue().compareTo(value) > 0) {
                                if (!curRef.compareAndSet(cur, cur.getRight().getReference(), Node.State.REM2.ordinal(), cur.getRight().getStamp())) {
                                    throw new RuntimeException("impossible: can't CAS right child");
                                }
                            } else {
                                if (!curRef.compareAndSet(cur, cur.getLeft().getReference(), Node.State.REM2.ordinal(), cur.getRight().getStamp())) {
                                    throw new RuntimeException("impossible: can't CAS left child");
                                }
                            }

                            removed = true;
                        } else {
                            if (!curRef.compareAndSet(cur, cur, Node.State.REM2.ordinal(), Node.State.CLEAN.ordinal())) {
                                throw new RuntimeException("impossible: can't back in CLEAN state");
                            }
                        }

                        if (!prevRef.compareAndSet(prev, prev, Node.State.REM1.ordinal(), Node.State.CLEAN.ordinal())) {
                            throw new RuntimeException("impossible: can't back in CLEAN state");
                        }

                        return removed;
                    }
                }
            }
        }
    }

    public boolean contains(T value) {
        Node<T> cur = rootRef.getReference();
        while (cur != null && cur.getValue() != value) {
            cur = decision(cur, value).getReference();
        }

        return cur != null;
    }

    public boolean isEmpty() {
        return rootRef.getReference() == null;
    }

    private static class Node<T extends Comparable<T>> {
        private final T value;
        private final AtomicStampedReference<Node<T>> left =
                new AtomicStampedReference<Node<T>>(null, State.CLEAN.ordinal());
        private final AtomicStampedReference<Node<T>> right =
                new AtomicStampedReference<Node<T>>(null, State.CLEAN.ordinal());

        public Node(T v) {
            value = v;
        }

        public Node(T v, Node<T> l, Node<T> r) {
            value = v;
            left.set(l, State.CLEAN.ordinal());
            right.set(r, State.CLEAN.ordinal());
        }

        public T getValue() {
            return value;
        }

        public AtomicStampedReference<Node<T>> getLeft() {
            return left;
        }

        public AtomicStampedReference<Node<T>> getRight() {
            return right;
        }

        public boolean isLeaf() {
            return left.getReference() == null;
        }

        enum State {
            CLEAN,
            ADD,
            REM1,
            REM2
        }
    }
}