package ru.spbau.concurrent;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.*;

public class LockFreeSetImplTest {
  @Test
  public void multipleThreadsTest() throws InterruptedException {
    LockFreeSet<Integer> lockFreeSet = new LockFreeSetImpl<>();
    List<Thread> threads = new ArrayList<>();
    for (int k = 0; k < 10; k++) {
      int threadId = k;
      threads.add(new Thread(() -> {
        List<Integer> threadValues = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
          threadValues.add(i * 10 + threadId);
        }
        for (Integer value: threadValues) {
          assertTrue(lockFreeSet.add(value));
        }
        for (Integer value: threadValues) {
          assertTrue(lockFreeSet.contains(value));
        }
        for (Integer value: threadValues) {
          assertTrue(lockFreeSet.remove(value));
        }
      }));
    }
    for (Thread thread: threads) {
      thread.start();
    }
    for (Thread thread: threads) {
      thread.join();
    }
    assertTrue(lockFreeSet.isEmpty());
  }
}
