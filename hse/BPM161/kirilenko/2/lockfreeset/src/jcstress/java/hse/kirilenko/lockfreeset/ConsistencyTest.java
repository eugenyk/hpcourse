package hse.kirilenko.lockfreeset;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true, true", expect = ACCEPTABLE, desc = "All values successfully added before queries.")
@Outcome(id = "true, false", expect = ACCEPTABLE, desc = "First added after query for second.")
@Outcome(id = "false, false", expect = ACCEPTABLE, desc = "None added before queries")
@Outcome(id = "false, true", expect = FORBIDDEN, desc = "Already have seen second add, but not the first add.")
@State
public class ConsistencyTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
        set.add(2);
    }

    @Actor
    public void actor2(ZZ_Result r) {
        r.r2 = set.contains(2);
        r.r1 = set.contains(1);
    }
}
