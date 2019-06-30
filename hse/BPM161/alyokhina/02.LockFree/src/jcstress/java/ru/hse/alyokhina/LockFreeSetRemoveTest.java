package ru.hse.alyokhina;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

@org.openjdk.jcstress.annotations.JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false", expect = Expect.FORBIDDEN)
@State
public class LockFreeSetRemoveTest {
    private final LockFreeSet<Integer> set;

    public LockFreeSetRemoveTest() {
        set = new LockFreeSetImpl<>();
        for (int i = 0; i < 10; i++) {
            set.add(i);
        }
    }

    @Actor
    public void actor1() {
        for (int i = 0; i < 5; i++) {
            set.remove(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 3; i < 7; i++) {
            set.remove(i);
        }
    }

    @Actor
    public void actor3() {
        for (int i = 9; i < 12; i++) {
            set.remove(i);
        }
    }

    @Arbiter
    public void arbiter(L_Result r) {
        boolean flag = true;
        final Set<Integer> correct = new HashSet<>(Arrays.asList(7, 8));
        for (int i = 0; i < 15; i++) {
            if (correct.contains(i) != set.contains(i)) {
                flag = false;
            }
        }
        r.r1 = flag;
    }
}
