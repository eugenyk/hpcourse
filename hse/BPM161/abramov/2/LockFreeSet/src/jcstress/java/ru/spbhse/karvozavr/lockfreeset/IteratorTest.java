package ru.spbhse.karvozavr.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

@JCStressTest
@Description("Test iterator correctness.")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Iterator is correct.")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Iteractor is incorrect.")
@State
public class IteratorTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private static final int ITERATIONS = 50;

    private Set<Integer> setFromIterator;

    @Actor
    public void adder1() {
        for (int j = 0; j < ITERATIONS; ++j) {
            set.add(j);
        }
    }

    @Actor
    public void adder2() {
        for (int j = 0; j < ITERATIONS; ++j) {
            set.add(j);
        }
    }

    @Actor
    public void iteratorGetter() {
        Iterator<Integer> iterator = set.iterator();
        setFromIterator = new HashSet<>();
        iterator.forEachRemaining(setFromIterator::add);
    }

    private boolean isCorrect() {
        int n = setFromIterator.size();
        for (int i = 0; i < n; i++) {
            if (!setFromIterator.contains(i)) {
                return false;
            }
        }
        return true;
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = isCorrect();
    }
}

