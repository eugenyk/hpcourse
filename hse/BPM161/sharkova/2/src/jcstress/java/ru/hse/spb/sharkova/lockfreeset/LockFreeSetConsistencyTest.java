package ru.hse.spb.sharkova.lockfreeset;


import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.LL_Result;

@Description("Operations consistency test for two threads adding and querying values.")
@Outcome(id = "true, true", expect = Expect.ACCEPTABLE, desc = "Values added before querying.")
@Outcome(id = "false, true", expect = Expect.ACCEPTABLE, desc = "First added after second value query.")
@Outcome(id = "false, false", expect = Expect.ACCEPTABLE, desc = "Querying before values were added.")
@Outcome(id = "true, false", expect = Expect.FORBIDDEN, desc = "Second value observed before first.")
@State
public class LockFreeSetConsistencyTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1(LL_Result r) {
        r.r1 = set.contains(2);
        r.r2 = set.contains(1);
    }

    @Actor
    public void actor2() {
        set.add(1);
        set.add(2);
    }
}
