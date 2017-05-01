import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

public class AtomicMarkableReferenceWithWaitFreeGet<T> {
    private class Value {
        final T ref;
        final boolean mark;

        private Value(T ref, boolean mark) {
            this.ref = ref;
            this.mark = mark;
        }
    }

    private final static AtomicReferenceFieldUpdater<AtomicMarkableReferenceWithWaitFreeGet, AtomicMarkableReferenceWithWaitFreeGet.Value> valueUpdater
            =  AtomicReferenceFieldUpdater.newUpdater(AtomicMarkableReferenceWithWaitFreeGet.class, AtomicMarkableReferenceWithWaitFreeGet.Value.class, "value");
    private volatile Value value;

    public AtomicMarkableReferenceWithWaitFreeGet(T initialRef, boolean initialMark) {
        value = new Value(initialRef, initialMark);
    }

    public boolean compareAndSet(T expectedReference, T newReference, boolean expectedMark, boolean newMark) {
        Value oldValue = value;
        if (oldValue.ref != expectedReference || oldValue.mark != expectedMark) {
            return false;
        }
        Value newValue = new Value(newReference, newMark);
        return valueUpdater.compareAndSet(this, oldValue, newValue);
    }

    public T get(boolean[] mark) {
        Value oldValue = value;
        mark[0] = oldValue.mark;
        return oldValue.ref;
    }
}
