package org.anstreth.queue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedDeque;

@StressCTest
public class LincheckTest {
    private Queue<Integer> queue = new ConcurrentLinkedDeque<>();

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
        LinChecker.check(LincheckTest.class);
    }
}
