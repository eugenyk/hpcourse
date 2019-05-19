package ru.hse.egorov;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true, false, true", expect = ACCEPTABLE)
@Outcome(id = "true, false, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, true", expect = FORBIDDEN)
@State
public class RemoveStressTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

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
        r.r3 = set.remove(1);
    }
}


