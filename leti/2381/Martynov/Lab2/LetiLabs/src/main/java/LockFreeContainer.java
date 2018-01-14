/*  Формальное определение lock-free объекта звучит так - разделяемый объект называется lock-free объектом
    (неблокируемым, non-blocking объектом), если он гарантирует, что некоторый поток закончит выполнение операции над
    объектом за конечное число шагов вне зависимости от результата работы других потоков (даже если эти другие потоки
    завершились крахом).*/

import java.util.concurrent.atomic.AtomicMarkableReference;

@SuppressWarnings("unchecked")
public class LockFreeContainer<T extends Comparable<T>> implements LockFreeSet<T> {

    private Node head = new Node();
    private Node tail = new Node();

    public LockFreeContainer() {
        head.key = Integer.MIN_VALUE;
        head.next = new AtomicMarkableReference<>(tail, false);

        tail.key = Integer.MAX_VALUE;
    }

    private Window find(int key) {
        Node pred, curr, succ;
        boolean[] marked = {false};
        boolean snip;
        retry:
        while (true) {
            pred = head;
            curr = (Node) pred.next.getReference();
            if (curr == tail)
                // ! if head == tail, return tail and head
                return new Window(pred, tail);
            while (true) {
                if (curr == tail)
                    // ! if on tail, return tail and pred tail.key > curr.key always
                    return new Window(pred, tail);
                succ = (Node) curr.next.get(marked);
                while (marked[0]) {
                    snip = pred.next.compareAndSet(curr, succ, false, false);
                    if (!snip)
                        continue retry;
                    curr = succ;
                    succ = (Node) curr.next.get(marked);
                }
                if (curr.key >= key)
                    return new Window(pred, curr);
                pred = curr;
                curr = succ;
            }
        }
    }

    @Override
    public boolean add(T value) {
        int key = value.hashCode();
        while (true) {
            Window window = find(key);
            Node pred = window.pred;
            Node curr = window.curr;
            if (curr.key == key)
                return false;
            else {
                Node node = new Node(value);
                node.next = new AtomicMarkableReference<>(curr, false);
                if (pred.next.compareAndSet(curr, node, false, false))
                    return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        int key = value.hashCode();
        boolean snip;
        while (true) {
            Window window = find(key);
            Node pred = window.pred;
            Node curr = window.curr;
            if (curr.key != key) {
                return false;
            } else {
                Node succ = (Node) curr.next.getReference();
                snip = curr.next.attemptMark(succ, true);
                if (!snip)
                    continue;
                pred.next.compareAndSet(curr, succ, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        boolean[] marked = {false};
        int key = value.hashCode();
        Node curr = head;
        while (curr.key < key) {
            curr = (Node) curr.next.getReference();

            if (curr == tail)
                //! if reached tail then does not contain value
                return false;

            curr.next.get(marked);
        }
        return (curr.key == key && !marked[0]);
    }

    @Override
    public boolean isEmpty() {
        Node succ;
        boolean[] marked = {false};
        while (true) {
            Node pred = head;
            Node curr = (Node) head.next.getReference();
            if (curr == tail)
                return true;
            succ = (Node) curr.next.get(marked);
            if (!marked[0])
                return false;

            pred.next.compareAndSet(curr, succ, true, false);
        }
    }
}
