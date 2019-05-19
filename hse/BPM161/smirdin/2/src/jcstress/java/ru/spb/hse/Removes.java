package ru.spb.hse;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, false", expect = ACCEPTABLE, desc = "Only 1 remove is successful")
@Outcome(id = "false, true", expect = ACCEPTABLE, desc = "Only 1 remove is successful")
@Outcome(id = "false, false", expect = ACCEPTABLE, desc = "Removes before add")
@State
public class Removes {
    private ILockFreeSet<Integer> set = new LockFreeSet<>();

//    @Actor
//    public void actor1() {
//        set.add(1);
//    }

    @Actor
    public void actor2(ZZ_Result r) {
        set.add(1);
        r.r1 = set.remove(1);
    }

    @Actor
    public void actor3(ZZ_Result r) {
        r.r2 = set.remove(1);
    }

}
