package ru.parallel;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Test success")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Test fail")
@State
public class TestAddAndContains {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private int numOfElements = 42;

    private ArrayList<Integer> range = new ArrayList<>();

    private ArrayList<Boolean> adds = new ArrayList<>();
    private ArrayList<Boolean> contains = new ArrayList<>();


    public TestAddAndContains() {
        for (int i = 0; i < numOfElements; i++) {
            set.add(i);
        }

        for (int i = 0; i < numOfElements; i++) {
            range.add(i);
        }
        Collections.shuffle(range);
    }

    private void addSome() {
        for (int i = 0; i < numOfElements; i++) {
            adds.add(set.add(range.get(i)));
        }
    }

    private void check() {
        for (int i = 0; i < numOfElements; i++) {
            contains.add(set.contains(range.get(i)));
        }
    }

    @Actor
    public void actor1() {
        addSome();
    }

    @Actor
    public void actor2() {
        check();
    }

    @Arbiter
    public void arbiter(L_Result r) {
        Boolean res = true;
        for (int i = 0; i < numOfElements; i++) {
            res &= (!adds.get(i) || contains.get(i));
        }

        r.r1 = res;
    }
}
