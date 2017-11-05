/*  Формальное определение lock-free объекта звучит так - разделяемый объект называется lock-free объектом
    (неблокируемым, non-blocking объектом), если он гарантирует, что некоторый поток закончит выполнение операции над
    объектом за конечное число шагов вне зависимости от результата работы других потоков (даже если эти другие потоки
    завершились крахом).*/

public class LockFreeContainer<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> head;
    private final Node<T> tail;

    LockFreeContainer() {
        tail = new Node<>(null);
        head = new Node<>(null);
        head.next.set(tail, false);
    }

    public boolean add(T value) {
        final Node<T> newNode = new Node<>(value);
        while (true) {
            final Node<T> prev = search(value);
            final Node<T> cur = getNext(prev);

            if (cur.value == value) {
                return false;
            } else {
                newNode.next.set(cur, false);
                if (prev.next.compareAndSet(cur, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    public boolean remove(T value) {
        while (true) {

            final Node<T> prev = search(value);
            final Node<T> cur = getNext(prev);

            if (cur.value != value) {
                return false;
            }
            final Node<T> next = cur.next.getReference();

            if (cur.next.compareAndSet(next, next, false, true)) {
                return true;
            }
        }
    }

    private Node<T> search(T value) {
        Node<T> prev;
        Node<T> cur;
        Node<T> next;
        boolean[] marked = {false};

        if (isEmpty()) {
            return head;
        }

        while (true) {
            prev = head;
            cur = prev.next.getReference();

            while (true) {
                next = cur.next.get(marked);

                while (marked[0]) {
                    if (!prev.next.compareAndSet(cur, next, false, false)) {
                        search(value);
                    }

                    cur = next;
                    next = cur.next.get(marked);
                }

                if (cur == tail || value.compareTo(cur.value) <= 0) {
                    return prev;
                }
                prev = cur;
                cur = next;
            }
        }
    }

    public boolean contains(T value) {
        final Node<T> prev = search(value);
        final Node<T> cur = getNext(prev);

        return (cur.value == value && !cur.next.isMarked());
    }

    public boolean isEmpty() {
        return head.next.getReference() == tail;
    }

    private Node<T> getNext(Node<T> prev) {
        return prev.next.getReference();
    }

    int getSize() {
        int i = 0;
        Node<T> prev;
        Node<T> cur;

        while (true) {
            prev = head;
            cur = prev.next.getReference();
            while (true) {
                i++;
                if (cur == tail) {
                    return i - 1;
                }
                cur = cur.next.getReference();
            }
        }
    }
}
