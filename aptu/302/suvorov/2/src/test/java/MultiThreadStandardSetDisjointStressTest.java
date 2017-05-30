import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class MultiThreadStandardSetDisjointStressTest extends MultiThreadSetDisjointStressTest {
    public MultiThreadStandardSetDisjointStressTest(int threads, int valsPerThread, int operations, boolean initializeFull) {
        super(threads, valsPerThread, operations, initializeFull);
    }

    @Override
    LockFreeSet<Integer> constructSet() {
        return new LockFreeSetStandardLibImpl<>();
    }
}
