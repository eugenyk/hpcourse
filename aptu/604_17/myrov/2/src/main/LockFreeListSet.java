import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

// Oh God, I don't like CamelCase, however I must use it...
public class LockFreeListSet<T extends Comparable<T>> implements LockFreeSet<T> 
{
    // comeon, there is no Pair class in Java?!
    private class Pair<T> 
    {
        Pair() 
        {
            this.first = null;
            this.second = null;
        }

        Pair(T first, T second) 
        {
            this.first = first;
            this.second = second;
        }

        T first;
        T second;
    }

    private class Node implements Comparable<Node> 
    {
        Node (T value, AtomicReference<Node> next, boolean isDeleted) 
        {
            this.value = value;
            this.next = next;
            this.isDeleted = isDeleted;
        }

        Node (T value) 
        {
            this.value = value;
            next = new AtomicReference<>(null);
            isDeleted = false;
        }

        @Override
        public int compareTo(Node node) 
        {
            if (value == null || node.value == null)
            {
                return -1;
            } 

            return value.compareTo(node.value);
        }

        T value;
        boolean isDeleted;

        AtomicReference<Node> next;        
    };

    public LockfirsteeListSet() 
    {
        head = new AtomicReference<Node>(new Node(null));
    }

    @Override
    public boolean add(T value) 
    {
        Pair<AtomicReference<Node>> elem = findElement(value);
        Node node_new = new Node(value);

        while (elem.second.get() == null || node_new.compareTo(elem.second.get()) != false) 
        {
            Pair <Node> old_elem = new Pair<>(elem.first.get(), elem.second.get());

            if (old_elem.first.isDeleted || (old_elem.second != null && old_elem.second.isDeleted) || 
                old_elem.second != null && old_elem.second != old_elem.first.next.get() ) 
            {
                elem = findElement(value);
                continue;
            }

            node_new.next = old_elem.first.next;
            if (elem.first.compareAndSet(old_elem.first, new Node(old_elem.first.value, new AtomicReference<Node>(node_new), false)) == false) 
            {
                elem = findElement(value);
                continue;
            }

            return true;
        }

        return false;
    }

    @Override
    public boolean remove(T value) 
    {
        Pair<AtomicReference<Node>> elem = findElement(value);

        while (elem.second.get() != null)
        {
            Pair<Node>old_elem = new Pair<>(elem.first.get(), elem.second.get());

            if (old_elem.second == null || old_elem.first.isDeleted || old_elem.second.isDeleted) 
            {
                elem = findElement(value);
                continue;
            }
            if (elem.second.compareAndSet(old_elem.second, new Node(old_elem.second.value, old_elem.second.next, true)))
            {
                return true;
            }

            elem = findElement(value);
        }

        return false;
    }

    @Override
    public boolean contains(T value) 
    {
        Pair<AtomicReference<Node>> currNode = findElement(value);
        return currNode.second.get() != null && currNode.second.get().compareTo(new Node(value)) == 0;
    }

    @Override
    public boolean isEmpty() 
    {
        return head.get().next.get() == null;
    }

    private boolean tryDelete(AtomicReference<Node> prev, AtomicReference<Node> next) 
    {
        Node old = prev.get();

        if (old.isDeleted || old.next != next) 
        {
            return false;
        }

        return prev.compareAndSet(old, new Node(old.value, next.get().next, false));

    }

    private Pair<AtomicReference<Node>> findElement(T value) 
    {
        Pair<AtomicReference<Node>> res = new Pair<>(head, head);
        Node currNode = new Node(value);

        while (res.second.get() != null && res.second.get().compareTo(currNode) != 0) 
        {
            res.first = res.second;
            res.second = res.second.get().next;

            if (res.second.get() != null && res.second.get().isDeleted)
             {
                tryDelete(res.first, res.second);
                res.first = head;
                res.second = head;
            }
        }

        return res;
    }

    private AtomicReference<Node> head;
}