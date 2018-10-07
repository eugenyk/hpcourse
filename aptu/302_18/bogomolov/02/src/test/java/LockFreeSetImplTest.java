import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.HandleExceptionAsResult;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.annotations.Reset;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.stress.StressCTest;
import org.junit.Test;

// This test uses 3 parallel threads and executes 1-4 operations in each
@StressCTest(iterations = 500, actorsPerThread = {"1:4", "1:4", "1:4"})
@Param(name = "value", gen = IntGen.class) // conf = "-10:10" by default
public class LockFreeSetImplTest {

    private LockFreeSet<Integer> set;

    // This method is invoked before every test invocation
    @Reset
    public void reset() {
        set = new LockFreeSetImpl<>();
    }

    @Operation
    public boolean add(@Param(name = "value") Integer value) {
        return set.add(value);
    }

    @Operation
    public boolean remove(@Param(name = "value") Integer value) {
        return set.remove(value);
    }

    @Operation
    public boolean contains(@Param(name = "value") Integer value) {
        return set.contains(value);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    // Use JUnit to run test
    @Test
    public void test() {
        LinChecker.check(LockFreeSetImplTest.class);
    }
}
