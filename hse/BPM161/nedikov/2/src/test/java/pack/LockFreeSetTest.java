package pack;

import org.junit.Before;
import org.junit.Test;
import pack.ILockFreeSet;
import pack.LockFreeSet;

import java.util.HashSet;
import java.util.List;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetTest {
  private ILockFreeSet<Integer> set;

  @Before
  public void createSet() {
    set = new LockFreeSet<>();
  }

  @Test
  public void testAdd() {
    assertTrue(set.add(1));
    assertTrue(set.add(2));
    assertTrue(set.add(3));
    assertFalse(set.add(3));
    assertFalse(set.add(2));
    assertTrue(set.add(-1));
  }

  @Test
  public void testRemove() {
    set.add(1);
    set.add(2);
    set.add(3);
    assertTrue(set.remove(1));
    assertTrue(set.remove(2));
    assertTrue(set.remove(3));
    assertFalse(set.remove(2));
    assertFalse(set.remove(4));
  }

  @Test
  public void testContains() {
    set.add(1);
    set.add(2);
    set.add(3);
    assertTrue(set.contains(1));
    assertTrue(set.contains(2));
    assertTrue(set.contains(3));
    assertFalse(set.contains(0));
    assertFalse(set.contains(4));
    assertTrue(set.contains(2));
  }

  @Test
  public void testIsEmpty() {
    assertTrue(set.isEmpty());
    set.add(1);
    assertFalse(set.isEmpty());
    set.remove(1);
    assertTrue(set.isEmpty());

  }

  @Test
  public void testIterator() {
    HashSet<Integer> realSet = new HashSet<>(List.of(1, 2, 3));
    set.add(1);
    set.add(2);
    set.add(3);
    for (var elem : set) {
      assertTrue(realSet.contains(elem));
    }

    set.remove(2);

    for (var elem : set) {
      assertTrue(realSet.contains(elem));
    }

    set.add(4);
    boolean containsAll = true;
    for (var elem : set) {
      containsAll &= realSet.contains(elem);
    }
    assertFalse(containsAll);
  }

}
