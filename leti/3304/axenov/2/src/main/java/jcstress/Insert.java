package jcstress;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

import LockFreePriorityQueue.*;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "0", expect = FORBIDDEN,  desc = "One insert lost.")
@Outcome(id = "1", expect = ACCEPTABLE, desc = "Both inserts.")
@State
public class Insert 
{
    private LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue();

    @Actor
    public void actor1() 
	{
        queue.add(1);
    }

    @Actor
    public void actor2() 
	{
        queue.offer(2);
    }

    @Arbiter
    public void arbiter(I_Result r) 
	{
		queue.remove();
		queue.poll();
        if(queue.isEmpty())
			r.r1 = 1;
		else
			r.r1 = 0;
    }
}
