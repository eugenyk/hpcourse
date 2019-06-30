package hse.kirilenko.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


@JCStressTest
@Description("Check add 40 elems from 4 threads")
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Add is correct")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Add is incorrect")
@State
public class AddTestUniqueElements {
    private LockFreeSet<Integer> lfs = new LockFreeSet<>();

    @Actor
    public void actor1() {
        for (int i = 0; i < 10; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 10; i < 20; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor3() {
        for (int i = 20; i < 30; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor4() {
        for (int i = 30; i < 40; i++) {
            lfs.add(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        boolean success = true;
        for (int i = 0; i < 40; i++) {
            success &= lfs.contains(i);
        }
        r.r1 = success;
    }
}