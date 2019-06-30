package ru.spbhse.karvozavr.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.LL_Result;

@JCStressTest
@Description("Test consistency of operations.")
@Outcome(id = "true, true", expect = Expect.ACCEPTABLE, desc = "Operations probably consistent.")
@Outcome(id = "false, true", expect = Expect.ACCEPTABLE, desc = "Operations probably consistent.")
@Outcome(id = "false, false", expect = Expect.ACCEPTABLE, desc = "Operations probably consistent.")
@Outcome(id = "true, false", expect = Expect.FORBIDDEN, desc = "Consistency is incorrect.")
@State
public class ConsistencyTest {
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
