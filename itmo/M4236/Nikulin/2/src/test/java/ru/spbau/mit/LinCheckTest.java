package ru.spbau.mit;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;
import ru.spbau.mit.PriorityQueueLockFreeImpl;

import java.util.PriorityQueue;
import java.util.Queue;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinCheckTest {
    private Queue<Integer> q = new PriorityQueueLockFreeImpl<>(false);
//    private Queue<Integer> q = new PriorityQueueLockFreeImplMok<>();

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

    @Operation
    public boolean isEmpty() {
        return q.isEmpty();
    }
    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }

//    @Override public boolean equals(Object o) { ... }
//    @Override public int hashCode() { ... }
}
