import java.util.concurrent.atomic.AtomicReference;

public class LockFreeTree  <T extends Comparable<T>> implements LockFreeSet<T> {
    private AtomicReference<Node> root = new AtomicReference<>(null);

    @Override
    public boolean add(T value) {
        while (true) {
            if (contains(value)) {
                return false;
            }

            Node newn = new Node(value);

            AtomicReference<Node> current = root;
            while (true) {
                if (current.get().x.compareTo(value) < 0) {
                    if (current.get().right.get() == null) {
                        break;
                    }
                    current.set(current.get().right.get());
                } else if (current.get().x.compareTo(value) > 0) {
                    if (current.get().left.get() == null) {
                        break;
                    }
                    current.set(current.get().left.get());
                } else {
                    return false;
                }
            }

            Node old = current.get();
            if (old.state != 0) {
                continue;
            }

            if (old.x.compareTo(value) < 0) {
                if (current.compareAndSet(old, new Node(old.x, old.state, old.left, new AtomicReference<Node>(newn)))) {
                    return true;
                }
                continue;
            }

            if (old.x.compareTo(value) > 0) {
                if (current.compareAndSet(old, new Node(old.x, old.state, new AtomicReference<Node>(newn), old.right))) {
                    return true;
                }
                continue;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            AtomicReference<Node> old = new AtomicReference<>(null);
            AtomicReference<Node> current = root;
            while (true) {
                old.set(current.get());
                if (current.get().x.compareTo(value) < 0) {
                    if (current.get().right.get() == null) {
                        return false;
                    }
                    current.set(current.get().right.get());
                } else if (current.get().x.compareTo(value) > 0) {
                    if (current.get().left.get() == null) {
                        return false;
                    }
                    current.set(current.get().left.get());
                } else {
                    break;
                }
            }

            Node oldCurVal = current.get();
            Node oldOld = old.get();

            if (oldCurVal.right == null) {
                if (oldCurVal.state != 0) {
                    continue;
                }

                if (oldOld == null) {
                    if (root.compareAndSet(oldCurVal, oldCurVal.left.get())) {
                        return true;
                    } else {
                        continue;
                    }
                }

                if (oldOld.state != 0) {
                    continue;
                }

                if (!old.compareAndSet(oldOld, new Node(oldOld.x, 1, oldOld.left, oldOld.right))) continue;
                if (!current.compareAndSet(oldCurVal, new Node(oldCurVal.x, 2, oldCurVal.left, oldCurVal.right))) {
                    old.compareAndSet(oldOld, new Node(oldOld.x, 0, oldOld.left, oldOld.right));
                    continue;
                }

                old.compareAndSet(new Node(oldOld.x, 1, oldOld.left, oldOld.right),
                        new Node(oldOld.x, 0, oldOld.left, oldCurVal.left));

                return true;
            } else {
                if (oldCurVal.state != 0) {
                    continue;
                }

                AtomicReference<Node> mr = findRight(oldCurVal.right);
                Node valMr = mr.get();

                if (valMr.state != 0) {
                    continue;
                }

                if (!mr.compareAndSet(valMr, new Node(valMr.x, 1, valMr.left, valMr.right)) ) {
                    continue;
                }

                if (!current.compareAndSet(oldCurVal, new Node(oldCurVal.x, 1, oldCurVal.left, oldCurVal.right))) {
                    mr.compareAndSet(new Node(valMr.x, 1, valMr.left, valMr.right),
                            new Node(valMr.x, 0, valMr.left, valMr.right));
                    continue;
                }

                mr.compareAndSet(new Node(valMr.x, 1, valMr.left, valMr.right),
                        new Node(valMr.x, 2, valMr.left, valMr.right));

                current.compareAndSet(new Node(oldCurVal.x, 1, oldCurVal.left, oldCurVal.right),
                        new Node(valMr.x, 0, oldCurVal.left, oldCurVal.right));

                return true;
            }
        }
    }

    private AtomicReference<Node> findRight(AtomicReference<Node> v) {
        if (v.get().right.get() == null) {
            return v;
        }
        return findRight(v.get().right);
    }

    @Override
    public boolean contains(T value) {
        return find(root, value);
    }

    @Override
    public boolean isEmpty() {
        if (root.get() == null) {
            return true;
        }
        return false;
    }

    private class Node {
        T x;
        int state;

        AtomicReference<Node> left;
        AtomicReference<Node> right;

        public Node(T value) {
            x = value;

            state = 0;
            left = new AtomicReference<>(null);
            right = new AtomicReference<>(null);
        }

        public Node(T value, int st, AtomicReference<Node> lt, AtomicReference<Node> rt) {
            x = value;
            state = st;
            left = lt;
            right = rt;
        }
    }


    private boolean find(AtomicReference<Node> root, T value) {
        if (root.get() == null) {
            return false;
        } else if (root.get().x.compareTo(value) == 0) {
            return true;
        } else if (root.get().x.compareTo(value) < 0) {
            return find(root.get().right, value);
        } else {
            return find(root.get().left, value);
        }
    }

}
