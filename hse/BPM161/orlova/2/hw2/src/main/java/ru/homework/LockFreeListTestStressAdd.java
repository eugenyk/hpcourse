package ru.homework;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZZZ_Result;

@JCStressTest

@Outcome(id = "false, true, true, true, true, true", expect = Expect.ACCEPTABLE,
        desc = "One update lost: atomicity failure.")
@State
public class LockFreeListTestStressAdd {

    LockFreeSet<Integer> set = new LockFreeList<>();

    @Actor
    public void actor1() {
        set.add(0);
    }

    @Actor
    public void actor2() {
        set.add(1);
    }

    @Actor
    public void actor3() {
        set.add(2);
    }

    @Actor
    public void actor4() {
        set.add(3);
    }

    @Actor
    public void actor5() {
        set.add(4);
    }

    @Arbiter
    public void arbiter(ZZZZZZ_Result r) {
        r.r1 = set.isEmpty();
        r.r2 = set.contains(1);
        r.r3 = set.contains(1);
        r.r4 = set.contains(1);
        r.r5 = set.contains(1);
        r.r6 = set.contains(1);
    }
}