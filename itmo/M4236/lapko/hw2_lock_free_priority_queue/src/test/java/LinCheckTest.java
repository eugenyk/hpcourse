import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.verifier.quiescent.QuiescentConsistencyVerifier;
import org.junit.Test;
import priority_q.LockFreePriorityQueue;

import java.util.PriorityQueue;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinCheckTest {
    private LockFreePriorityQueue<Integer> q = new LockFreePriorityQueue<>();

    @Operation
    public boolean offer(@Param(name = "key") int key) {
        return q.offer(key);
    }

    @Operation
    public Integer peek() {
        return q.peek();
    }

//    @Operation
//    public Integer pool() {
//        return q.poll();
//    }

//    @Operation
//    public Integer size() {
//        return q.size();
//    }
    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }

//    @Override public boolean equals(Object o) { ... }
//    @Override public int hashCode() { ... }
}