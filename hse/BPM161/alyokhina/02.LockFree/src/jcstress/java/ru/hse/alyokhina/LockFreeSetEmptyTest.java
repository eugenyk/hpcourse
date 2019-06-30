package ru.hse.alyokhina;


import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZ_Result;

@org.openjdk.jcstress.annotations.JCStressTest
@Outcome(id = "true, true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false, true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false, false", expect = Expect.FORBIDDEN)
@Outcome(id = "true, false", expect = Expect.FORBIDDEN)
@State
public class LockFreeSetEmptyTest {
    private final LockFreeSet<Integer> set;

    public LockFreeSetEmptyTest() {
        set = new LockFreeSetImpl<>();
        for (int i = 0; i < 3; i++) {
            set.add(i);
        }
    }

    @Actor
    public void actor1(ZZ_Result r) {
        r.r1 = set.isEmpty();
    }

    @Actor
    public void actor2() {
        for (int i = 0; i < 3; i++) {
            set.remove(i);
        }
    }

    @Arbiter
    public void arbiter(ZZ_Result r) {
        r.r2 = set.isEmpty();
    }
}
