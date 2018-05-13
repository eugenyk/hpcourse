package ru.spbau.eshcherbin.hpcourse.lockfreeset;

import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest {

  private LockFreeSet<Integer> set;

  @Before
  public void setUp() {
    set = new LockFreeSetImpl<>();
  }

  // Single-thread tests

  @Test
  public void testIsEmpty() {
    assertTrue(set.isEmpty());
  }

  @Test
  public void testNotEmptyAfterAdd() {
    assertTrue(set.add(117));
    assertFalse(set.isEmpty());
  }

  @Test
  public void testContainsAfterAdd() {
    assertTrue(set.add(117));
    assertTrue(set.contains(117));
    assertFalse(set.contains(225));
  }

  @Test
  public void testAddAndRemove() {
    assertTrue(set.add(117));
    assertTrue(set.remove(117));
    assertFalse(set.contains(117));
    assertTrue(set.isEmpty());
  }

  @Test
  public void testSeveralAddAndDelete() {
    for (int i = 0; i < 100; ++i) {
      assertTrue(set.add(i));
    }
    for (int i = 99; i >= 0; --i) {
      assertFalse(set.add(i));
    }
    for (int i = 0; i < 100; ++i) {
      assertTrue(set.contains(i));
    }
    for (int i = 99; i >= 0; --i) {
      assertTrue(set.remove(i));
    }
    for (int i = 0; i < 100; ++i) {
      assertFalse(set.remove(i));
    }
    assertTrue(set.isEmpty());
  }

  // Multi-thread test

  @Test
  public void testAll() throws InterruptedException {
    List<Thread> addThreads = new ArrayList<>();
    for (int i = 0; i < 100; ++i) {
      int iCopy = i; // need effectively final variable
      addThreads.add(new Thread(() -> set.add(iCopy / 2)));
    }
    for (Thread thread : addThreads) {
      thread.start();
    }
    for (Thread thread : addThreads) {
      thread.join();
    }

    assertFalse(set.isEmpty());
    for (int i = 0; i < 50; ++i) {
      assertTrue(set.contains(i));
    }

    List<Thread> removeThreads = new ArrayList<>();
    for (int i = 0; i < 100; ++i) {
      int iCopy = i; // need effectively final variable
      removeThreads.add(new Thread(() -> set.remove(iCopy / 2)));
    }
    for (Thread thread : removeThreads) {
      thread.start();
    }
    for (Thread thread : removeThreads) {
      thread.join();
    }

    assertTrue(set.isEmpty());
    for (int i = 0; i < 50; ++i) {
      assertFalse(set.contains(i));
    }
  }
}