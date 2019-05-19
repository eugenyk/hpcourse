package pack;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class LockFreeSet<T extends Comparable<T>> implements ILockFreeSet<T> {
  private SetNode<T> head = new SetNode<>();
  private AtomicInteger listSize = new AtomicInteger(0);

  @Override
  public boolean add(T value) {
    if (value == null) return false;
    var valNode = new SetNode<>(value);
    var node = head;
    while (!node.next.compareAndSet(null, valNode)) {
      node = node.next.get();
      if (node.has(value)) {
        return false;
      }
    }
    listSize.incrementAndGet();
    return true;
  }

  @Override
  public boolean remove(T value) {
    if (value == null) return false;
    var node = head.next.get();
    while (node != null) {
      if (node.elem.compareTo(value) == 0) {
        if (node.deleted.compareAndSet(false, true)) {
          return true;
        }
      }
      node = nextNode(node);
    }
    return false;
  }

  @Override
  public boolean contains(T value) {
    if (value == null) return false;
    int n = listSize.get();
    var node = head.next.get();
    for (int i = 0; node != null && i < n; i++) {
      if (node.has(value)) {
        return true;
      }
      node = nextNode(node);
    }
    return false;
  }

  // delete deleted nodes
  private SetNode<T> nextNode(SetNode<T> node) {
    var nodeNext = node.next.get();
    if (nodeNext == null) return null;
    if (!nodeNext.deleted.get()) return nodeNext;
    if (node.next.compareAndSet(nodeNext, nodeNext.next.get())) {
      listSize.decrementAndGet();
    }
    return node.next.get();
  }

  @Override
  public boolean isEmpty() {
    return !iterator().hasNext();
  }

  @Override
  public Iterator<T> iterator() {
    var prev = snapshot();
    while (true) {
      var cur = snapshot();
      if (compareSnapshots(prev, cur)) {
        return cleanupSnapshot(prev).iterator();
      }
      prev = cur;
    }
  }

  private List<T> cleanupSnapshot(List<SnapNode<T>> a) {
    var result = new ArrayList<T>();
    for (var x : a) {
      if (!x.deleted) {
        result.add(x.link.elem);
      }
    }
    return result;
  }

  private boolean compareSnapshots(List<SnapNode<T>> a, List<SnapNode<T>> b) {
    int n = a.size();
    if (n != b.size()) return false;
    for (int i = 0; i < n; i++) {
      if (!a.get(i).eq(b.get(i))) return false;
    }
    return true;
  }

  private List<SnapNode<T>> snapshot() {
    var result = new ArrayList<SnapNode<T>>();
    var node = head.next.get();
    while (node != null) {
      result.add(node.copyLW());
      node = node.next.get();
    }
    return result;
  }


  private static class SetNode<T extends Comparable<T>> {
    AtomicReference<SetNode<T>> next = new AtomicReference<>(null); // всегда указывает на следующего соседа или на null
    AtomicBoolean deleted = new AtomicBoolean(false); // меняется только с false на true, но не обратно
    final T elem; // null только в голове

    SetNode(T elem) {
      this.elem = elem;
    }

    SetNode() {
      this(null);
    }

    SnapNode<T> copyLW() {
      return new SnapNode<T>(this, deleted.get());
    }

    boolean has(T value) {
      return !deleted.get() && elem.compareTo(value) == 0;
    }
  }


  private static class SnapNode<T extends Comparable<T>> {
    final boolean deleted;
    final SetNode<T> link;

    SnapNode(SetNode<T> link, boolean deleted) {
      this.link = link;
      this.deleted = deleted;
    }

    boolean eq(SnapNode<T> node) {
      return deleted == node.deleted && link == node.link;
    }
  }
}
