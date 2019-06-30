package hse.kirilenko.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Description("Test remove")
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Remove is correct")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Remove is incorrect")
@State
public class RemoveTestUniqueElements {
    private LockFreeSet<Integer> lfs = new LockFreeSet<>();

    public RemoveTestUniqueElements() {
        for (int i = 0; i < 40; i++) {
            lfs.add(i);
        }
    }

    @Actor
    public void actor1() {
        for (int i = 0; i < 10; i++) {
            lfs.remove(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 10; i < 20; i++) {
            lfs.remove(i);
        }
    }

    @Actor
    public void actor3() {
        for (int i = 20; i < 30; i++) {
            lfs.remove(i);
        }
    }

    @Actor
    public void actor4() {
        for (int i = 30; i < 40; i++) {
            lfs.remove(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = lfs.isEmpty();
    }
}