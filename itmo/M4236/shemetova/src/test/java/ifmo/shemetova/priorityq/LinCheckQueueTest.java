package ifmo.shemetova.priorityq;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@Param(name = "element", gen = IntGen.class, conf = "1:1000")
@StressCTest
public class LinCheckQueueTest {
    private HPPriorityQueue<Integer> queue = new HPPriorityQueue<>();


    @Operation
    public void offer(@Param(name = "element") int e) {
        queue.offer(e);
    }

    @Operation
    public Integer poll() {
        return queue.poll();
    }

    @Test
    public void runTest() {
        LinChecker.check(LinCheckQueueTest.class);
    }
}