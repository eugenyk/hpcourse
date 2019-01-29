package priorityQueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.Queue;

@StressCTest
public class LockFreePriorityQueueTestLinCheck {
    private Queue<Integer> queue = new LockFreePriorityQueue<>();

    @Operation
    public void offer(Integer x) {
        queue.offer(x);
    }

    @Operation
    public Integer poll() {
        return queue.poll();
    }

    @Test
    public void runTest() {
        LinChecker.check(LockFreePriorityQueueTestLinCheck.class);
    }
}