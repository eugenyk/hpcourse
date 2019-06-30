package ru.hse.alyokhina;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.L_Result;

@org.openjdk.jcstress.annotations.JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false", expect = Expect.FORBIDDEN)
@State
public class LockFreeSetAddTest {
    private final LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void actor1() {
        for (int i = 0; i < 5; i++) {
            set.add(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 20; i < 25; i++) {
            set.add(i);
        }
    }

    @Arbiter
    public void arbiter(L_Result r) {
        boolean flag = true;
        for (int i = 0; i < 5; i++) {
            if (!set.contains(i)) {
                flag = false;
            }
        }
        for (int i = 20; i < 25; i++) {
            if (!set.contains(i)) {
                flag = false;
            }
        }
        r.r1 = flag;
    }
}
