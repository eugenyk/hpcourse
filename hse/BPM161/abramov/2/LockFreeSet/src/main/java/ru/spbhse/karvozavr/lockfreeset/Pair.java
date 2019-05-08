package ru.spbhse.karvozavr.lockfreeset;

public class Pair<T> {
    public Pair(T first, T second) {
        this.first = first;
        this.second = second;
    }

    public final T first;
    public final T second;
}
