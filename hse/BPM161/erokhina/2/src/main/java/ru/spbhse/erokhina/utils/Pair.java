package ru.spbhse.erokhina.utils;

public class Pair<T, N> {
    private T first;
    private N second;

    public Pair(T first, N second) {
        this.first = first;
        this.second = second;
    }

    public T getFirst() {
        return first;
    }

    public void setFirst(T first) {
        this.first = first;
    }

    public N getSecond() {
        return second;
    }

    public void setSecond(N second) {
        this.second = second;
    }
}
