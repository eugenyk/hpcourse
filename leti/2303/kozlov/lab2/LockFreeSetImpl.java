import com.sun.istack.internal.NotNull;

import java.util.concurrent.atomic.AtomicReference;

/**
 * Created by xd720p on 29.10.2017.
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>{

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    @Override
    public boolean add(T value) {
        return false;
    }

    @Override
    public boolean remove(T value) {
        return false;
    }

    @Override
    public boolean contains(T value) {
        return false;
    }

    @Override
    public boolean isEmpty() {
        return this.head.get() == null;
    }

    private class Node{
        @NotNull
        private final T value;
        @NotNull
        private final Node nextNode;

        public Node(@NotNull T value, @NotNull Node nextNode) {
            this.value = value;
            this.nextNode = nextNode;
        }
    }
}
