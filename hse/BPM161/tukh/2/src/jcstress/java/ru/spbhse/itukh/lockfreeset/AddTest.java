package ru.spbhse.itukh.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

@JCStressTest
@Description("Simple add test with two actors")
@Outcome(id = "both", expect = Expect.ACCEPTABLE, desc = "All right")
@Outcome(id = "first", expect = Expect.FORBIDDEN, desc = "Not all right")
@Outcome(id = "second", expect = Expect.FORBIDDEN, desc = "Not all right")
@Outcome(id = "none", expect = Expect.FORBIDDEN, desc = "Completely right")
@State
public class AddTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(2);
        set.add(3);
        set.add(9);
    }

    @Actor
    public void actor2() {
        set.add(5);
        set.add(6);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        boolean first = set.contains(2) && set.contains(3) && set.contains(9);
        boolean second = set.contains(5) && set.contains(6);

        if (first && second) {
            r.r1 = "both";
        } else if (first) {
            r.r1 = "first";
        } else if (second) {
            r.r1 = "second";
        } else {
            r.r1 = "none";
        }
    }
}
