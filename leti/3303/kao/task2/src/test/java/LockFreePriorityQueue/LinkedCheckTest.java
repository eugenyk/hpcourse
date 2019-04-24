package LockFreePriorityQueue;
import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;
import LockFreePriorityQueue.LockFreePriorityQueue;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinkedCheckTest {
    private LockFreePriorityQueue<Integer> q = new LockFreePriorityQueue<>();

    @Operation
    public boolean offer(@Param(name = "key") int key) {
        return q.offer(key);
    }

    @Operation
    public Integer peek() {
        return q.peek();
    }

    @Operation
    public Integer pool() {
        return q.poll();
    }

    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }

}