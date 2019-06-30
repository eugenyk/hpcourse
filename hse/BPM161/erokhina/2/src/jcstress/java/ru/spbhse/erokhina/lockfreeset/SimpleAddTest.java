package ru.spbhse.erokhina.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

@JCStressTest
@Outcome(id = "false, false, true, true", expect = Expect.ACCEPTABLE,  desc = "Correct additions.")
@Outcome(id = "false, false, false, false", expect = Expect.ACCEPTABLE_INTERESTING,  desc = "Set doesn't contain both elements.")
@Outcome(id = "false, false, true, false", expect = Expect.ACCEPTABLE_INTERESTING,  desc = "Set doesn't contain second element.")
@Outcome(id = "false, false, false, true", expect = Expect.ACCEPTABLE_INTERESTING,  desc = "Set doesn't contain first elements.")
@State
public class SimpleAddTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void actor1(ZZZZ_Result r) {
        set.add(0);
        r.r1 = set.add(0);
    }

    @Actor
    public void actor2(ZZZZ_Result r) {
        set.add(1);
        r.r2 = set.add(1);
    }

    @Arbiter
    public void arbiter(ZZZZ_Result r) {
        r.r3 = set.contains(0);
        r.r4 = set.contains(1);
    }
}
