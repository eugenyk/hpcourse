package ru.spbau.concurrent;

import org.jetbrains.annotations.NotNull;

import java.util.concurrent.atomic.AtomicMarkableReference;

/** Implementation of {@link LockFreeSet}. */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
  private final Node<T> tail = new Node<>(null, null);
  private final Node<T> head = new Node<>(null, tail);

  @Override
  public boolean add(@NotNull T value) {
    Node<T> cur = head;
    Node<T> pred = cur;
    while (true) {
      if (cur.next.isMarked()) {
        forwardReference(pred, cur);
        return add(value);
      }
      int comparison = compareValueToNodeValue(value, cur);
      if (comparison == 0) {
        return false;
      } else if (comparison < 0) {
        Node<T> newNode = new Node<>(value, cur);
        if (pred.next.compareAndSet(cur, newNode, false, false)) {
          return true;
        } else {
          return add(value);
        }
      } else {
        pred = cur;
        cur = cur.next.getReference();
      }
    }
  }

  @Override
  public boolean remove(@NotNull T value) {
    Node<T> cur = head;
    Node<T> pred = cur;
    while (true) {
      if (cur.next.isMarked()) {
        forwardReference(pred, cur);
        return remove(value);
      }
      int comparison = compareValueToNodeValue(value, cur);
      if (comparison == 0) {
        Node<T> next = cur.next.getReference();
        if (cur.next.compareAndSet(next, next, false, true)) {
          forwardReference(pred, cur);
          return true;
        } else {
          return remove(value);
        }
      } else if (comparison < 0) {
        return false;
      } else {
        pred = cur;
        cur = cur.next.getReference();
      }
    }
  }

  @Override
  public boolean contains(@NotNull T value) {
    Node<T> cur = head;
    Node<T> pred = cur;
    while (true) {
      if (cur.next.isMarked()) {
        forwardReference(pred, cur);
        return contains(value);
      }
      int comparison = compareValueToNodeValue(value, cur);
      if (comparison == 0) {
        return true;
      } else if (comparison < 0) {
        return false;
      } else {
        pred = cur;
        cur = cur.next.getReference();
      }
    }
  }

  @Override
  public boolean isEmpty() {
    Node<T> cur = head.next.getReference();
    if (cur.next.isMarked()) {
      forwardReference(head, cur);
      return isEmpty();
    }
    return cur == tail;
  }

  private int compareValueToNodeValue(@NotNull T value, @NotNull Node<T> node) {
    // Any value is greater than the head and is less than the tail.
    return node == head ? 1 : (node == tail ? -1 : value.compareTo(node.value));
  }

  private static<T extends Comparable<T>> void forwardReference(
      @NotNull Node<T> pred, @NotNull Node<T> cur) {
    pred.next.compareAndSet(cur, cur.next.getReference(), false, false);
  }

  private static class Node<T extends Comparable<T>> {
    final T value;
    /**
     * Contains the reference to the next element and the flag meaning whether the current
     * node was removed.
     */
    final AtomicMarkableReference<Node<T>> next;

    Node(T value, Node<T> next) {
      this.value = value;
      this.next = new AtomicMarkableReference<>(next, false);
    }
  }
}
