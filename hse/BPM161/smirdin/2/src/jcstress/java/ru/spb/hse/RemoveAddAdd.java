package ru.spb.hse;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true", expect = ACCEPTABLE, desc = "All pased")
@Outcome(id = "false, false, true", expect = ACCEPTABLE, desc = "Too early delete")
@Outcome(id = "true, false, false", expect = ACCEPTABLE, desc = "Too early delete")
@Outcome(id = "true, true, false", expect = ACCEPTABLE, desc = "Too early add")
@Outcome(id = "false, true, true", expect = ACCEPTABLE, desc = "Too early add")
@State
public class RemoveAddAdd {
    private ILockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1(ZZZ_Result r) {
        r.r1 = set.add(1);
    }

    @Actor
    public void actor2(ZZZ_Result r) {
        r.r2 = set.remove(1);
    }

    @Actor
    public void actor3(ZZZ_Result r) {
        r.r3 = set.add(1);
    }
}
