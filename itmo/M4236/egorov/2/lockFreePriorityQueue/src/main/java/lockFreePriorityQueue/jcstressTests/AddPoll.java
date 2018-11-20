
package lockFreePriorityQueue.jcstressTests;

import java.util.ArrayList;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;
import lockFreePriorityQueue.PriorityQueue;
import lockFreePriorityQueue.PriorityQueueImpl;

@JCStressTest
@Outcome(id = "2, 1, 2", expect = Expect.ACCEPTABLE)
@State
public class AddPoll {
	
	PriorityQueue<Integer> q = new PriorityQueueImpl<>();
	ArrayList<Integer> q1 = new ArrayList<>();
	
    @Actor
    public void actor1() {
        q.add(1);
        q.add(2);
    }
    
    @Actor
    public void reader1(III_Result r) {
    	while (q1.size() < 2) {
    		Integer i = q.poll();
    		if (i != null) {
    			q1.add(i);
    			r.r1 = q1.size();
    		}
    	}
    }

    @Arbiter
    public void testContent(III_Result r) {
    	r.r1 = q1.size();
    	r.r2 = q1.get(0);
    	r.r3 = q1.get(1);
    }
    
}
