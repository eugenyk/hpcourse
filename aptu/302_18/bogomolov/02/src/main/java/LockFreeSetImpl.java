public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

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
        return false;
    }
}
