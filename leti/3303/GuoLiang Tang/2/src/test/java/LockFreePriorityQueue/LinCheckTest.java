package LockFreePriorityQueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;
//import priority_q.LockFreePriorityQueue;

import java.util.PriorityQueue;
import java.util.Queue;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinCheckTest {
    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue();

    @Operation
    public void insert(@Param(name = "key") int key) {
        queue.insert(key);
    }

    @Operation
    public Integer extractMinimum() {
        return queue.extractMinimum();
    }

    @Operation
    public boolean isEmpty() {
        return queue.isEmpty();
    }

    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }

}