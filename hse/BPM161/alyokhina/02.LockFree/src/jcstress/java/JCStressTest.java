import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.infra.results.Z_Result;

@org.openjdk.jcstress.annotations.JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false", expect = Expect.FORBIDDEN)
@State
public class JCStressTest {

    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    public JCStressTest() {
        for (int i = 20; i < 30; ++i) {
            set.add(i);
        }
    }

    @Actor
    public void add1() {
        for (int i = 0; i < 10; ++i) {
            set.add(i);
        }
    }

    @Actor
    public void add2() {
        for (int i = 10; i < 20; ++i) {
            set.add(i);
        }
    }

    @Actor
    public void remove1() {
        for (int i = 25; i < 35; ++i) {
            set.remove(i);
        }
    }

    @Actor
    public void remove2() {
        for (int i = 35; i < 45; ++i) {
            set.remove(i);
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = true;
        for (int i = 0; i < 25; i++) {
            r.r1 &= set.contains(i);
        }
        for (int i = 25; i < 50; i++) {
            r.r1 &= !set.contains(i);
        }
    }
}