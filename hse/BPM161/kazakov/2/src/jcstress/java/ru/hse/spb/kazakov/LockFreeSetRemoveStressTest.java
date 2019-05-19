package ru.hse.spb.kazakov;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

@JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "All elements are removed.")
@Outcome(id= "false",expect=Expect.FORBIDDEN,desc="Some elements are still in the set.")
@State
public class LockFreeSetRemoveStressTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    public LockFreeSetRemoveStressTest() {
        for (int i = 0; i <= 30; i++) {
            set.add(i);
        }
    }

    @Actor
    public void remove1() {
        for (int i = 0; i <= 20; i++) {
            set.remove(i);
        }
    }

    @Actor
    public void remove2() {
        for (int i = 10; i <= 30; i++) {
            set.remove(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = set.isEmpty();
    }
}