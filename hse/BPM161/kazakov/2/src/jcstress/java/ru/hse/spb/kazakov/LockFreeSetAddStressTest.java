package ru.hse.spb.kazakov;

import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.infra.results.Z_Result;

@JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "All elements are added.")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Some elements are missing.")
@State
public class LockFreeSetAddStressTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add1() {
        for (int i = 0; i <= 20; i++) {
            set.add(i);
        }
    }

    @Actor
    public void add2() {
        for (int i = 21; i <= 40; i++) {
            set.add(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        boolean result = true;
        for (int i = 0; i <= 40; i++) {
            result &= set.contains(i);
        }
        r.r1 = result;
    }
}