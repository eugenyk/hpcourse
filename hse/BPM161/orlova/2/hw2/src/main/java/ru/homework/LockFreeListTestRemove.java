package ru.homework;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

@JCStressTest

@Outcome(id = "true, true, true", expect = Expect.ACCEPTABLE,
        desc = "One update lost: atomicity failure.")
@Outcome(id = "true, false, false", expect = Expect.ACCEPTABLE,
        desc = "One update lost: atomicity failure.")
@Outcome(id = "true, false, true", expect = Expect.ACCEPTABLE,
        desc = "One update lost: atomicity failure.")
@Outcome(id = "true, true, false", expect = Expect.ACCEPTABLE,
        desc = "One update lost: atomicity failure.")
@State
public class LockFreeListTestRemove {
    LockFreeSet<Integer> set = new LockFreeList<>();

    @Actor
    public void actor1(ZZZ_Result result) {
        result.r1 = set.add(0);
    }

    @Actor
    public void actor2(ZZZ_Result result) {
        result.r2 = set.remove(0);
    }

    @Actor
    public void actor3(ZZZ_Result result) {
        result.r3 = set.isEmpty();
    }
}
