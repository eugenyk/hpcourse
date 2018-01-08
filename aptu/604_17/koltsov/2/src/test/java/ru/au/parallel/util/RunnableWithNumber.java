package ru.au.parallel.util;

abstract public class RunnableWithNumber implements Runnable {
    protected int runnableNumber;

    protected RunnableWithNumber(int x) {
        runnableNumber = x;
    }
}
