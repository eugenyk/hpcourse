package ru.spbhse.itukh.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;
import org.openjdk.jcstress.infra.results.Z_Result;

@JCStressTest
@Description("Simple add and remove test with three actors")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "All right")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Troubles")
@State
public class AddAndRemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(2);
        set.add(3);
        set.add(1);
        set.remove(2);
        set.remove(3);
        set.remove(1);
    }

    @Actor
    public void actor2() {
        set.add(5);
        set.remove(5);
    }

    @Actor
    public void actor3() {
        set.add(4);
        set.remove(4);
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = set.isEmpty();
    }
}