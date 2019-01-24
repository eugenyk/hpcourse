package leti._3303.faber;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@Param(name = "value", gen = IntGen.class, conf = "1:1000")
@StressCTest
public class LockFreePriorityQueueLinearizationTest {

    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Operation
    public void offer(@Param(name = "value") int value) {
        queue.offer(value);
    }

    @Operation
    public Integer poll() {
        return queue.poll();
    }

    @Operation
    public Integer peek() {
        return queue.peek();
    }

    @Operation
    public Boolean isEmpty() {
        return queue.isEmpty();
    }

    @Test
    public void test() {
        LinChecker.check(LockFreePriorityQueueLinearizationTest.class);
    }
}