import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.Queue;

@Param(name = "key", gen = IntGen.class, conf = "1:1000")
@StressCTest
public class PriorityQueueLincheckTest {

        private Queue<Integer> queue = new LockFreePriorityQueue<>();

        @Operation
        public void offer(Integer e) {
            queue.offer(e);
        }

        @Operation
        public Integer poll() {
            return queue.poll();
        }

        @Operation
        public Integer peek() {
            return queue.peek();
        }

        @Test
        public void runTest() {
            LinChecker.check(PriorityQueueLincheckTest.class);
        }
}
