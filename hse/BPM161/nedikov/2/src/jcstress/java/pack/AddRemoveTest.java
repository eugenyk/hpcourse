package pack;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "1324 or etc")
@Outcome(id = "true, false, true, false", expect = ACCEPTABLE, desc = "1234")
@Outcome(id = "true, false, false, true", expect = ACCEPTABLE, desc = "1243")
@Outcome(id = "false, true, true, false", expect = ACCEPTABLE, desc = "2134")
@Outcome(id = "false, true, false, true", expect = ACCEPTABLE, desc = "2143")
@Outcome(id = "true, false, false, false", expect = ACCEPTABLE, desc = "3412 or etc")
@Outcome(id = "false, true, false, false", expect = ACCEPTABLE, desc = "3421 or etc")
@Outcome(id = "true, true, true, false", expect = ACCEPTABLE, desc = "1342 or etc")
@Outcome(id = "true, true, false, true", expect = ACCEPTABLE, desc = "1432 or etc")
@State
public class AddRemoveTest {
  private LockFreeSet<Integer> set = new LockFreeSet<>();

  @Actor
  public void actor1(ZZZZ_Result r) {
    r.r1 = set.add(1);
  }

  @Actor
  public void actor2(ZZZZ_Result r) {
    r.r2 = set.add(1);
  }

  @Actor
  public void actor3(ZZZZ_Result r) {
    r.r3 = set.remove(1);
  }

  @Actor
  public void actor4(ZZZZ_Result r) {
    r.r4 = set.remove(1);
  }
}
