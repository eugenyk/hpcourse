package lockFreePriorityQueue.jcstressTests;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.Arbiter;
import org.openjdk.jcstress.annotations.Expect;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.IIII_Result;
import lockFreePriorityQueue.PriorityQueue;
import lockFreePriorityQueue.PriorityQueueImpl;

@JCStressTest
@Outcome(id = "1, 1, 2, 3", expect = Expect.ACCEPTABLE)
@State
public class ConcAddConcPoll {
	PriorityQueue<Integer> q_start = new PriorityQueueImpl<>();
	PriorityQueue<Integer> q_end = new PriorityQueueImpl<>();
	
    @Actor
    public void actor1() {
        q_start.add(1);
        q_start.add(2);
    }

    @Actor
    public void actor2() {
        q_start.add(1);
        q_start.add(3);
    }

    void readerRoutine() {
    	while (q_end.size() < 4) {
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
    public void testContent(IIII_Result r) {
    	r.r1 = q_end.poll();
    	r.r2 = q_end.poll();
    	r.r3 = q_end.poll();
    	r.r4 = q_end.poll();
    	q_start.poll();
    }
}
