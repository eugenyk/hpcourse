package pack;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@StressCTest
public class LockFreeSetTestLinCheck {
  private LockFreeSet<Integer> set = new LockFreeSet<>();

  @Test
  public void lockFreeSetTestLinCheck() {
    LinChecker.check(LockFreeSetTestLinCheck.class);
  }

  @Operation
  public boolean add(int key) {
    return set.add(key);
  }

  @Operation
  public boolean remove(int key) {
    return set.add(key);
  }


  @Operation
  public boolean contains(int key) {
    return set.contains(key);
  }

  @Operation
  public boolean isEmpty() {
    return set.isEmpty();
  }

  @Operation
  public void iterator() {
    set.iterator().forEachRemaining(val -> {});
  }
}