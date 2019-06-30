package pack;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true", expect = ACCEPTABLE)
@State
public class RemoveTest {
  private LockFreeSet<Integer> set = new LockFreeSet<>();

  RemoveTest() {
    set.add(1);
    set.add(2);
  }

  @Actor
  public void actor1(ZZZ_Result r) {
    r.r1 = set.remove(1);
  }

  @Actor
  public void actor2(ZZZ_Result r) {
    r.r2 = set.remove(2);
  }

  @Arbiter
  public void arbiter(ZZZ_Result r) {
    r.r3 = set.isEmpty();
  }
}