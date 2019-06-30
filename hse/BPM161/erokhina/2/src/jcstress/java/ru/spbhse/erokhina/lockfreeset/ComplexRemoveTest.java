package ru.spbhse.erokhina.lockfreeset;

import org.apache.commons.collections4.IteratorUtils;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.ArrayList;
import java.util.List;

@JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE,  desc = "Correct deletions.")
@Outcome(id = "false", expect = Expect.ACCEPTABLE_INTERESTING,  desc = "Incorrect deletions.")
@State
public class ComplexRemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();
    private List<Integer> expectedResult = new ArrayList<>();

    ComplexRemoveTest() {
        for (int i = 0; i <= 30; i++) {
            set.add(i);
        }

        for (int i = 21; i <= 30; i++) {
            expectedResult.add(i);
        }
    }

    @Actor
    public void actor1() {
        for (int i = 0; i <= 15; i++) {
            set.remove(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 10; i <= 20; i++) {
            set.remove(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        List<Integer> realResult = IteratorUtils.toList(set.iterator());
        r.r1 = realResult.equals(expectedResult);
    }
}
