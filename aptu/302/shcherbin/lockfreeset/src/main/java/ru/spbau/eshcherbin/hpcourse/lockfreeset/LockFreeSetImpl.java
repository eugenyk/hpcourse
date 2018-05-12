package ru.spbau.eshcherbin.hpcourse.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
  private final Node<T> head = new Node<>(null, new AtomicMarkableReference<>(null, false));

  @Override
  public boolean add(T value) {
    Node<T>[] leftRightNodesHolder = new Node[2];
    Node<T> leftNode;
    Node<T> rightNode;

    do {
      search(value, leftRightNodesHolder);
      leftNode = leftRightNodesHolder[0];
      rightNode = leftRightNodesHolder[1];

      if (rightNode != null && rightNode.getValue().equals(value)) {
        return false;
      }
      Node<T> newNode = new Node<>(value, new AtomicMarkableReference<>(rightNode, false));
      if (leftNode.getNext().compareAndSet(rightNode, newNode, false, false)) {
        return true;
      }
    } while (true);
  }

  @Override
  public boolean remove(T value) {
    Node<T>[] leftRightNodesHolder = new Node[2];
    boolean[] markHolder = new boolean[1];
    Node<T> leftNode;
    Node<T> rightNode;
    Node<T> rightNodeNextValue;
    boolean rightNodeNextMark;

    do {
      search(value, leftRightNodesHolder);
      leftNode = leftRightNodesHolder[0];
      rightNode = leftRightNodesHolder[1];

      if (rightNode == null || !rightNode.getValue().equals(value)) {
        return false;
      }
      rightNodeNextValue = rightNode.getNext().get(markHolder);
      rightNodeNextMark = markHolder[0];
      if (!rightNodeNextMark) {
        if (rightNode.getNext().compareAndSet(rightNodeNextValue, rightNodeNextValue, false, true)) {
          break;
        }
      }
    } while (true);
    if (!leftNode.getNext().compareAndSet(rightNode, rightNodeNextValue, false, false)) {
      search(rightNode.getValue(), leftRightNodesHolder);
    }
    return true;
  }

  @Override
  public boolean contains(T value) {
    Node<T>[] leftRightNodesHolder = new Node[2];
    search(value, leftRightNodesHolder);

    Node<T> rightNode = leftRightNodesHolder[1];
    return rightNode != null && rightNode.getValue().equals(value);
  }

  @Override
  public boolean isEmpty() {
    Node<T> first = head.getNext().getReference();
    while (first != null && first.getNext().isMarked()) {
      Node<T> next = first.getNext().getReference();
      head.getNext().compareAndSet(first, next, false, false);
      first = head.getNext().getReference();
    }
    return first == null;
  }

  private void search(T value, Node<T> leftRightNodesHolder[]) {
    boolean[] markHolder = new boolean[1];
    Node<T> leftNode = null;
    Node<T> rightNode;
    Node<T> leftNodeNextValue = null;

    do {
      Node<T> current = head;
      Node<T> currentNextValue = current.next.get(markHolder);
      boolean currentNextMark = markHolder[0];

      do {
        if (!currentNextMark) {
          leftNode = current;
          leftNodeNextValue = currentNextValue;
        }
        current = currentNextValue;
        if (current == null) {
          break;
        }
        currentNextValue = current.getNext().get(markHolder);
        currentNextMark = markHolder[0];
      } while (currentNextMark || current.getValue().compareTo(value) < 0);
      rightNode = current;

      if (leftNodeNextValue == rightNode) {
        if (rightNode != null && rightNode.getNext().isMarked()) {
          continue;
        } else {
          leftRightNodesHolder[0] = leftNode;
          leftRightNodesHolder[1] = rightNode;
          return;
        }
      }

      if (leftNode.getNext().compareAndSet(leftNodeNextValue, rightNode, false, false)) {
        if (rightNode == null || !rightNode.getNext().isMarked()) {
          leftRightNodesHolder[0] = leftNode;
          leftRightNodesHolder[1] = rightNode;
          return;
        }
      }
    } while (true);
  }

  private static class Node<T> {
    private T value;
    private AtomicMarkableReference<Node<T>> next;

    public Node(T value, AtomicMarkableReference<Node<T>> next) {
      this.value = value;
      this.next = next;
    }

    public T getValue() {
      return value;
    }

    public AtomicMarkableReference<Node<T>> getNext() {
      return next;
    }
  }
}