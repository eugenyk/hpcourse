package chirukhin.lab2;

import chirukhin.lab2.priorityqueue.LockFreePriorityQueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;

import org.junit.Test;

import java.util.NoSuchElementException;

@StressCTest()
public class LinCheckTest {
    private LockFreePriorityQueue<Integer> q = new LockFreePriorityQueue<>();

    @Operation
    public void add(@Param(gen = IntGen.class) int value) {
        q.add(value);
    }

    @Operation
    public void offer(@Param(gen = IntGen.class) int value) {
        q.offer(value);
    }

    @Operation
    public Object poll() {
        return q.poll();
    }

    @Operation(handleExceptionsAsResult = NoSuchElementException.class)
    public Object remove() {
        return q.remove();
    }

    @Operation
    public Object peek() {
        return q.peek();
    }

    @Operation(handleExceptionsAsResult = NoSuchElementException.class)
    public Object element() {
        return q.element();
    }

    @Test
    public void test() {
        LinChecker.check(LinCheckTest.class);
    }
}
