package ru.parallel;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Add success")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Add fail")
@State
public class TestAdd2 {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private int numOfElements = 42;
    private AtomicInteger numOfSuccess = new AtomicInteger(0);

    private void addSome() {
        ArrayList<Integer> range = new ArrayList<>();

        for (int i = 0; i < numOfElements; i++) {
            range.add(i);
        }

        Collections.shuffle(range);

        for (int i = 0; i < numOfElements; i++) {
            if (set.add(range.get(i))) {
                numOfSuccess.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor1() {
        addSome();
    }

    @Actor
    public void actor2() {
        addSome();
    }

    @Actor
    public void actor3() {
        addSome();
    }

    @Actor
    public void actor4() {
        addSome();
    }

    @Actor
    public void actor5() {
        addSome();
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = numOfSuccess.get() == numOfElements;
    }
}
