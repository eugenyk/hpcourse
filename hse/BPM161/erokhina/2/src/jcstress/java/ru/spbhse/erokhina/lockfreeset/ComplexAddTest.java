package ru.spbhse.erokhina.lockfreeset;

import org.apache.commons.collections4.IteratorUtils;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.ArrayList;
import java.util.List;

@JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE,  desc = "Correct additions.")
@Outcome(id = "false", expect = Expect.ACCEPTABLE_INTERESTING,  desc = "Incorrect additions.")
@State
public class ComplexAddTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();
    private List<Integer> expectedResult = new ArrayList<>();

    ComplexAddTest() {
        for (int i = 0; i <= 30; i++) {
            expectedResult.add(i);
        }
    }

    @Actor
    public void actor1() {
        for (int i = 30; i >= 0; i -= 2) {
            set.add(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 29; i >= 0; i -= 2) {
            set.add(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        List<Integer> realResult = IteratorUtils.toList(set.iterator());
        r.r1 = realResult.equals(expectedResult);
    }
}

