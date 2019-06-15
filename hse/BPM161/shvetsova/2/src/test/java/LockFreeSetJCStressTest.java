import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "19", expect = Expect.ACCEPTABLE)
@State
public class LockFreeSetJCStressTest {

    private LockFreeSetInterface<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void add1() {
        for (int i = 0; i < 10; ++i) {
            set.add(i);
        }
    }

    @Actor
    public void add2() {
        for (int i = 0; i < 10; ++i) {
            set.add(-i);
        }
    }

    @Arbiter
    public void arbiter(I_Result r) {
        r.r1 = 0;
        for (int i = -10; i < 10; i++) {
            if (set.contains(i)) {
                r.r1 += 1;
            }
        }
    }
}