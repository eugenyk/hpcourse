package ru.hse.spb.sharkova.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@JCStressTest
@Description("Two threads add values simultaneously; check iterator correctness.")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Correct execution.")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Incorrect execution.")
@State
public class LockFreeSetIteratorTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private static final int size = 50;

    private List<Integer> values;

    @Actor
    public void actor1() {
        for (int i = 0; i < size; i++) {
            set.add(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 0; i < size; i++) {
            set.add(i);
        }
    }

    @Actor
    public void iteratorGetter() {
        Iterator<Integer> iterator = set.iterator();
        values = new ArrayList<>();
        iterator.forEachRemaining(values::add);
    }

    private boolean executedCorrectly() {
        for (int i = 0; i < values.size(); i++) {
            if (!values.contains(i)) {
                return false;
            }
        }
        return true;
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = executedCorrectly();
    }
}
