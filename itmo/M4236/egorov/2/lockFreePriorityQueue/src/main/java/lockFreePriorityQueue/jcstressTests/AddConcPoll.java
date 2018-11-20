
package lockFreePriorityQueue.jcstressTests;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;

import lockFreePriorityQueue.PriorityQueue;
import lockFreePriorityQueue.PriorityQueueImpl;


@JCStressTest
@Outcome(id = "1, 2", expect = Expect.ACCEPTABLE)
@State
public class AddConcPoll {
	
	PriorityQueue<Integer> q_start = new PriorityQueueImpl<>();
	PriorityQueue<Integer> q_end = new PriorityQueueImpl<>();
	
    @Actor
    public void actor1() {
        q_start.add(1);
        q_start.add(2);
    }

    void readerRoutine() {
    	while (q_end.size() < 2) {
    		Integer i = q_start.poll();
    		if (i != null) {
    			q_end.add(i);
    		}
    	}
    }

    @Actor
    public void reader1() {
    	readerRoutine();
    }

    @Actor
    public void reader2() {
    	readerRoutine();
    }

    @Arbiter
    public void testContent(II_Result r) {
    	r.r1 = q_end.poll();
    	r.r2 = q_end.poll();
    }
    
}
