package me.eranik.lockfree;

public interface Set<T extends Comparable<T>> {
    /**
     * Add a key to a set in a lock-free style
     *
     * @param value key value
     * @return false if set contains value, true otherwise
     */
    boolean add(T value);


    /**
     * Delete a key from a set in a lock-free style
     *
     * @param value key value
     * @return false if key was not found, true if key was successfully removed
     */
    boolean remove(T value);


    /**
     * Check if set contains a key in a lock-free style
     *
     * @param value key value
     * @return true if set contains value, false otherwise
     */
    boolean contains(T value);


    /**
     * Check if set is empty in a lock-free style
     *
     * @return true if set is empty, false otherwise
     */
    boolean isEmpty();

    /**
     * Returns lock-free iterator for a set
     *
     * @return new iterator instance for a set
     */
    java.util.Iterator<T> iterator();
}
