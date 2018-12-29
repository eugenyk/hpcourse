import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E>{


    public class Node <E extends Comparable<E>> {
        public E data;
        public AtomicMarkableReference<Node<E>> next;

        Node(E new_data){
            data = new_data;
            next = new AtomicMarkableReference<>(null, false);
        }

        Node (E new_data, Node<E> new_next){
            data = new_data;
            next = new AtomicMarkableReference<>(new_next, false);
        }
    }

    private AtomicMarkableReference<Node<E>> head;
    private AtomicMarkableReference<Node<E>> tail;
    private AtomicInteger node_counter;

    LockFreePriorityQueue(){

        Node<E> _tail = new Node<>(null);
        Node<E> _head = new Node<>(null,_tail);
        head = new AtomicMarkableReference<>(_head, false);
        tail = new AtomicMarkableReference<>(_tail, false);
        node_counter = new AtomicInteger(0);



    }
    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return node_counter.get();
    }

    @Override
    public boolean offer(E e) {
        Node<E> newNode = new Node<>(e);
        while (true)
        {

            Node<E> previous = head.getReference();
            Node<E> current = previous.next.getReference();

            boolean[] to_delete = {false};
            Node<E> next = current.next.get(to_delete);

            while (true)
            {
                if (to_delete[0])
                {
                    if (previous.next.compareAndSet(current, next, false, false))
                    {
                        node_counter.decrementAndGet();
                    }
                }
                else if (next == null || e.compareTo(current.data) < 0)
                {
                    break;
                }
                previous = current;
                current = current.next.getReference();
                next = current.next.get(to_delete);
            }
            newNode.next.set(current, false);
            if (previous.next.compareAndSet(current, newNode, false, false))
            {
                node_counter.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public E poll() {
        boolean[] to_delete = {false};
        Node<E> first = null;
        Node<E> current = null;
        Node<E> next = null;
        Node<E> last = null;
        while (true)
        {
            first = head.getReference();
            current = first.next.getReference();
            next = current.next.get(to_delete);
            last = tail.getReference();

            if (to_delete[0]) //Если можем помочь потоку удалить элемент
            {
                if (first.next.compareAndSet(current, next, false, false))
                {
                    node_counter.decrementAndGet();
                }
            }
            else if (current != last) //Если очередь пуста
            {
                E value = current.data;
                if (current.next.compareAndSet(next, next, false, true)) //Проверяем, нет ли изменений и отмечаем узел на удаление
                {
                    if (first.next.compareAndSet(current, next, false, false))
                    {
                        node_counter.decrementAndGet();
                    }
                    return value;
                }
            }
            else
            {
                return null;
            }
        }
    }

    @Override
    public E peek() {
        Node<E> first;
        Node<E> current;
        Node<E> next;
        Node<E> last;

        boolean[] currentMayBeDeleted = {false};
        while (true)
        {
            first = head.getReference();
            current = first.next.getReference();
            next = current.next.get(currentMayBeDeleted);
            last = tail.getReference();

            if (currentMayBeDeleted[0])
            {
                if (first.next.compareAndSet(current, next, false, false))
                {
                    node_counter.decrementAndGet();
                }
            }
            else if (current != last)
            {
                return current.data;
            }
            else {
                return null;
            }
        }
    }

    @Override
    public  boolean isEmpty(){
        Node<E> first = null;
        Node<E> current = null;
        Node<E> next = null;
        Node<E> last = null;

        boolean[] to_delete = {false};
        first = head.getReference();
        current = first.next.getReference();
        last = tail.getReference();

        if (current == last){
            return true;
        } else {
            next = current.next.get(to_delete);
            if (to_delete[0])
            {
                if (first.next.compareAndSet(current, next, false, false))
                {
                    node_counter.decrementAndGet();
                }
            }
        }
        return false;
    }
}
