package lockfree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true, false, true", expect = ACCEPTABLE)
@Outcome(id = "false, true, true", expect = ACCEPTABLE)
@Outcome(id = "true, false, false", expect = ACCEPTABLE)
@Outcome(id = "false, true, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, true", expect = ACCEPTABLE)
@Outcome(id = "false, false, false", expect = FORBIDDEN)
@Outcome(id = "false, false, true", expect = FORBIDDEN)
@Outcome(id = "true, true, false", expect = FORBIDDEN)
@State
public class AddAddRemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add1(ZZZ_Result r) {
        r.r1 = set.add(1);
    }

    @Actor
    public void add2(ZZZ_Result r) {
        r.r2 = set.add(1);
    }

    @Actor
    public void delete(ZZZ_Result r) {
        r.r3 = set.remove(1);
    }
}
