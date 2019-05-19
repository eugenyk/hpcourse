package ru.spb.hse;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Empty set")
@State
public class CheckIsEmpty {
    private ILockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
        set.remove(1);
    }

    @Actor
    public void actor2() {
        set.add(2);
        set.remove(2);
    }

    @Actor
    public void actor3() {
        set.add(3);
        set.remove(3);
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = set.isEmpty();
    }
}