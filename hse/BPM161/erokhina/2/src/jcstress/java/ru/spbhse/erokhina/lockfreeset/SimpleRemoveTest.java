package ru.spbhse.erokhina.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZZZ_Result;

@JCStressTest
@Outcome(id = "true, true, false, false, false, false", expect = Expect.ACCEPTABLE,  desc = "actor1 removed both elements.")
@Outcome(id = "false, false, true, true, false, false", expect = Expect.ACCEPTABLE,  desc = "actor2 removed both elements.")
@Outcome(id = "true, false, false, true, false, false", expect = Expect.ACCEPTABLE,  desc = "actor1 removed 0, actor2 - 1.")
@Outcome(id = "false, true, true, false, false, false", expect = Expect.ACCEPTABLE,  desc = "actor1 removed 1, actor2 - 0.")
@State
public class SimpleRemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    SimpleRemoveTest() {
        set.add(0);
        set.add(1);
    }

    @Actor
    public void actor1(ZZZZZZ_Result r) {
        r.r1 = set.remove(0);
        r.r2 = set.remove(1);
    }

    @Actor
    public void actor2(ZZZZZZ_Result r) {
        r.r3 = set.remove(0);
        r.r4 = set.remove(1);
    }

    @Arbiter
    public void arbiter(ZZZZZZ_Result r) {
        r.r5 = set.contains(0);
        r.r6 = set.contains(1);
    }
}
