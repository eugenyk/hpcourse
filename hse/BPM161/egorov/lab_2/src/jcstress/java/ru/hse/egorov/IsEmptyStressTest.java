package ru.hse.egorov;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true, true", expect = ACCEPTABLE)
@State
public class IsEmptyStressTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    IsEmptyStressTest() {
        set.add(1);
        set.add(2);
        set.add(3);
    }

    @Actor
    public void actor1(ZZZZ_Result r) {
        r.r1 = set.remove(1);
    }

    @Actor
    public void actor2(ZZZZ_Result r) {
        r.r2 = set.remove(2);
    }

    @Actor
    public void actor3(ZZZZ_Result r) {
        r.r3 = set.remove(3);
    }


    @Arbiter
    public void arbiter(ZZZZ_Result r) {
        r.r4 = set.isEmpty();
    }
}