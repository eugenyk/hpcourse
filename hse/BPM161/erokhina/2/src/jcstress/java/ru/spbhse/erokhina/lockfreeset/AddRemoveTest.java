package ru.spbhse.erokhina.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

@JCStressTest
@Outcome(id = "true, true, true, true", expect = Expect.ACCEPTABLE,  desc = "actor1 acted, then actor2 acted (or vice versa).")
@Outcome(id = "true, true, false, false", expect = Expect.ACCEPTABLE,  desc = "a1 added, a2 added, a1 deleted, a2 deleted.")
@Outcome(id = "true, false, false, true", expect = Expect.ACCEPTABLE,  desc = "a1 added, a2 added, a2 deleted, a1 deleted.")
@Outcome(id = "false, false, true, true", expect = Expect.ACCEPTABLE,  desc = "a2 added, a1 added, a2 deleted, a1 deleted.")
@Outcome(id = "false, true, true, false", expect = Expect.ACCEPTABLE,  desc = "a2 added, a1 added, a1 deleted, a2 deleted.")
@State
public class AddRemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void actor1(ZZZZ_Result r) {
        r.r1 = set.add(0);
        r.r2 = set.remove(0);
    }

    @Actor
    public void actor2(ZZZZ_Result r) {
        r.r3 = set.add(0);
        r.r4 = set.remove(0);
    }
}
