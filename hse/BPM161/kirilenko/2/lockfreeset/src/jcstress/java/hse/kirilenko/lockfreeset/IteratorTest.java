package hse.kirilenko.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import java.util.*;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Description("Test iterator")
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Iterator is correct")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Iterator is incorrect")
@State
public class IteratorTest {
    private LockFreeSet<Integer> lfs = new LockFreeSet<>();
    private Set<Integer> result;

    @Actor
    public void actor1() {
        for (int i = 0; i < 10; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 0; i < 10; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor3() {
        for (int i = 0; i < 10; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor4() {
        for (int i = 0; i < 10; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void iteratorGetter() {
        Iterator<Integer> iterator = lfs.iterator();
        result = new HashSet<>();

        while(iterator.hasNext()) {
            result.add(iterator.next());
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        boolean success = true;
        int n = result.size();

        for (int i = 0; i < result.size(); i++) {
            success &= result.contains(i);

        }

        r.r1 = success;
    }
}