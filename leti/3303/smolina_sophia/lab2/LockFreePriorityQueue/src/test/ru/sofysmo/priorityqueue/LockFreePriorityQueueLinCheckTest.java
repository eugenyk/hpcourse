package ru.sofysmo.priorityqueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import org.junit.Test;

@StressCTest
public class LockFreePriorityQueueLinCheckTest {
    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Operation
    public boolean offer(int e) {
        return queue.offer(e);
    }

    @Operation
    public Integer pool() {
        return queue.poll();
    }

    @Operation
    public Integer peek() {
        return queue.peek();
    }

    @Test
    public void runTest() {
        LinChecker.check(LockFreePriorityQueueLinCheckTest.class);
    }
}
/*import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@StressCTest
public class LockFreePriorityQueueLinCheckTest {
    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();;

    @Operation
    public Boolean offer(Integer e) {
        return queue.offer(e);
    }

    @Operation
    public Integer poll() {
        return queue.poll();
    }

    @Operation
    public boolean isEmpty() {
        return queue.isEmpty();
    }

    @Test
    public void test() {
        LinChecker.check(LockFreePriorityQueueLinCheckTest.class);
    }
}*/