package pack;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true", expect = ACCEPTABLE)
@State
public class ConcurrentAddTest {
  private LockFreeSet<Integer> set = new LockFreeSet<>();

  @Actor
  public void actor1() {
    set.add(1);
  }

  @Actor
  public void actor2() {
    set.add(2);
  }

  @Actor
  public void actor3() {
    set.add(3);
  }

  @Arbiter
  public void arbiter(ZZZ_Result r) {
    r.r1 = set.contains(1);
    r.r2 = set.contains(2);
    r.r3 = set.contains(3);
  }
}