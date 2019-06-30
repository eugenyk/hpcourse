package ru.spbhse.itukh.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

@JCStressTest
@Description("Simple add test with two actors")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "All right")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Not all right")
@State
public class IsEmptyTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(2);
        set.add(2);
        set.remove(2);
        set.remove(2);
    }

    @Actor
    public void actor2() {
        set.add(5);
        set.remove(5);
        set.add(1);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = !set.isEmpty();
    }
}
