import java.util.Collections;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicReference;

class Pair<T1, T2> {
    T1 first;
    T2 second;

    Pair(T1 first, T2 second) {
        this.first = first;
        this.second = second;
    }
}

class NodeInfo<T extends Comparable<T>> {
    volatile Node<T> next;
    volatile boolean marked;

    NodeInfo(Node<T> next, boolean marked) {
        this.next = next;
        this.marked = marked;
    }
}

class Node<T extends Comparable<T>> {
    volatile T key;
    AtomicReference<NodeInfo<T>> info;

    Node(T key, NodeInfo<T> info) {
        this.key = key;
        this.info = new AtomicReference<>(info);
    }
}

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head = new Node<>(null, new NodeInfo<T>(null, false));

    LockFreeSetImpl()
    { }

    private Pair<Node<T>, Node<T>> find(T value)
    {
        restart: while (true) {
            Node<T> prev = head;
            Node<T> curr = head.info.get().next;

            while (true) {
                if (curr == null)
                    break;

                NodeInfo<T> oldInfo = prev.info.get();
                if (curr.info.get().marked) {
                    NodeInfo<T> newInfo = new NodeInfo<>(curr.info.get().next, false);
                    if (oldInfo.next == curr && !oldInfo.marked && prev.info.compareAndSet(oldInfo, newInfo)) {
                        curr = prev.info.get().next;
                        continue;
                    }
                    else
                        continue restart;
                }

                if (value != null && curr.key.compareTo(value) < 0) {
                    prev = curr;
                    curr = prev.info.get().next;
                }
                else
                    break;
            }

            return new Pair<>(prev, curr);
        }
    }

    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value, new NodeInfo<T>(null, false));

        while (true) {
            Pair<Node<T>, Node<T>> found = find(value);
            Node<T> prev = found.first;
            Node<T> curr = found.second;

            if (curr != null && curr.key.compareTo(value) == 0)
                return false;
            else {
                NodeInfo<T> oldInfo = prev.info.get();
                if (oldInfo.next == curr && !oldInfo.marked) {
                    newNode.info.get().next = curr;
                    NodeInfo<T> newInfo = new NodeInfo<>(newNode, false);
                    if (prev.info.compareAndSet(oldInfo, newInfo))
                        return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Pair<Node<T>, Node<T>> found = find(value);
            Node<T> curr = found.second;

            if (curr != null && curr.key.compareTo(value) == 0) {
                NodeInfo<T> oldInfo = curr.info.get();

                if (oldInfo.marked)
                    continue;

                NodeInfo<T> newInfo = new NodeInfo<>(oldInfo.next, true);
                if (curr.info.compareAndSet(oldInfo, newInfo))
                    return true;
            }
            else {
                return false;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        Pair<Node<T>, Node<T>> found = find(value);
        Node<T> curr = found.second;
        return curr != null && curr.key.compareTo(value) == 0;
    }

    @Override
    public boolean isEmpty() {
        Pair<Node<T>, Node<T>> found = find(null);
        Node<T> curr = found.second;

        return curr == null;
    }

    static final int N = 1000;
    static final int M = 100;
    static Object[] locks;

    static void run(Set<Integer> set1, LockFreeSet<Integer> set2) {
        Random random = new Random();
        while (true) {
            int value = random.nextInt(N);
            int op = random.nextInt(3);
            boolean res1 = false;
            boolean res2 = false;

            synchronized (locks[value]) {
                if (op == 0) {
                    res1 = set1.contains(value);
                    res2 = set2.contains(value);
                }
                else if (op == 1) {
                    res1 = set1.add(value);
                    res2 = set2.add(value);
                }
                else if (op == 2) {
                    res1 = set1.remove(value);
                    res2 = set2.remove(value);
                }
            }

            if (res1 != res2)
                throw new RuntimeException("LOL");
        }
    }

    public static void main(String[] args) throws InterruptedException {
        locks = new Object[N];
        for (int i = 0; i < N; i++)
            locks[i] = new Object();

        final Set<Integer> set1 = Collections.newSetFromMap(new ConcurrentHashMap<Integer, Boolean>());
        final LockFreeSet<Integer> set2 = new LockFreeSetImpl<>();

        Thread last = null;
        for (int i = 0; i < M; i++) {
            last = new Thread(() -> run(set1, set2));
            last.start();
        }
        last.join();
    }
}
