package lockFreePriorityQueue;

import org.junit.Test;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;

@StressCTest
public class LinCheckTest {
	private PriorityQueue<Integer> q = new LockFreePriorityQueue<>();

	@Operation
	public Integer poll() {
		return q.poll();
	}
	
	@Operation
	public Integer peek() {
		return q.peek();
	}

	@Operation
	public boolean isEmpty() {
		return q.isEmpty();
	}
	/*
	@Operation
	public int size() {
		return q.size();
	}
	*/
	@Operation
	public boolean offer(@Param(gen = IntGen.class) Integer val) {
		return q.offer(val);
	}
	
	@Test
	public void runTest() {
		LinChecker.check(LinCheckTest.class);
	}
}
