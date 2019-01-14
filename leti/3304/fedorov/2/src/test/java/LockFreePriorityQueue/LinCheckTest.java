package LockFreePriorityQueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Iterator;
import java.util.NoSuchElementException;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LinCheckTest {
    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue();

	@Operation
    public boolean offer(@Param(name = "key") int key) {
        return queue.offer(key);
    }
	
	@Operation
    public Integer poll() {
        return queue.poll();
    }

    @Operation
    public boolean isEmpty() {
        return queue.isEmpty();
    }
	
	@Operation
    public Integer iteratorNext() {
		Iterator iterator = queue.iterator();
		try {
			return (Integer)iterator.next();
		}
		catch(NoSuchElementException e) {
			return null;
		}
    }
	
	@Operation
    public boolean iteratorHasNext() {
		Iterator iterator = queue.iterator();
        return iterator.hasNext();
    }

	@Operation
    public Integer peek() {
        return queue.peek();
    }
	
	@Operation
    public void clear() {
        queue.clear();
    }
	
	@Operation
    public boolean contains(@Param(name = "key") int key) {
        return queue.contains(key);
    }
	
	@Operation
    public int size() {
        return queue.size();
    }
	
    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }

}