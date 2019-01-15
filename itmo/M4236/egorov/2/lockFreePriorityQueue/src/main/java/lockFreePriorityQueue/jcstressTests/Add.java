
package lockFreePriorityQueue.jcstressTests;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;

import lockFreePriorityQueue.PriorityQueue;
import lockFreePriorityQueue.LockFreePriorityQueue;


@JCStressTest
@Outcome(id = "1, 2, 2", expect = Expect.ACCEPTABLE)
@State
public class Add {
	
	PriorityQueue<Integer> q = new LockFreePriorityQueue<>();
	
    @Actor
    public void actor1() {
        q.add(1);
    }

    @Actor
    public void actor2() {
        q.add(2);
    }

    @Arbiter
    public void testContent(III_Result r) {
    	r.r3 = q.size();
    	r.r1 = q.poll();
    	r.r2 = q.poll();
    }
}
