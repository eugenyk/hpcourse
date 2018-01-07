package ru.spbau.mit;

public class LockFreeSetException extends RuntimeException {
    LockFreeSetException(String message) {
        super(message);
    }
}
