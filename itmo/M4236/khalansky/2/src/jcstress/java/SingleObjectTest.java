import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "5", expect = Expect.ACCEPTABLE, desc = "Queue was used.")
@State
public class SingleObjectTest {

    LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    volatile int result = 0;

    @Actor
    public void actor1() {
        queue.offer(5);
    }

    @Actor
    public void actor2() {
        Integer v = queue.poll();
        if (v != null) {
            result += v;
        }
    }

    @Arbiter
    public void arbiter(I_Result r) {
        r.r1 = result;
    }

}
