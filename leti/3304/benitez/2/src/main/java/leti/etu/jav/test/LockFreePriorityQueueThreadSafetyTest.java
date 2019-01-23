/*
 * Copyright (c) 2017, Red Hat Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Oracle nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
package leti.etu.jav.test;

import leti.etu.jav.queue.LockFreePriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.IntResult1;
import org.openjdk.jcstress.infra.results.IntResult2;
import org.openjdk.jcstress.infra.results.IntResult3;

public class LockFreePriorityQueueThreadSafetyTest
{
    @State
    public static class QueueState {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();
    }
    
	@JCStressTest
	@Outcome(id = "2, 123, 321", expect = Expect.ACCEPTABLE, desc = "returns 2, 123 and 321")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
	public static class EnqSizeTest {

	    @Actor
	    public void actor1(QueueState q) {
	    	q.queue.add(123);
	    }

	    @Actor
	    public void actor2(QueueState q) {
	    	q.queue.add(321);
	    }
	    
        @Arbiter
        public void arbiter(QueueState q, IntResult3 r) {
        	int size = q.queue.size();
        	Integer result2 = q.queue.poll();
        	Integer result3 = q.queue.poll();
        	r.r1 = size;
            r.r2 = (result2 == null ? -1 : result2);
            r.r3 = (result3 == null ? -1 : result3);
        }

	}
	
	@JCStressTest
	@Outcome(id = "123, 321", expect = Expect.ACCEPTABLE, desc = "returns 123 and 321")
	@Outcome(id = "321, 123", expect = Expect.ACCEPTABLE, desc = "returns 321 and 123")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
	public static class EnqDeqTest {

	    @Actor
	    public void actor1(QueueState q, IntResult2 r) {
	    	q.queue.add(123);
	    	Integer result = q.queue.poll();
	    	r.r1 = (result == null ? -1 : result);
	    }

	    @Actor
	    public void actor2(QueueState q, IntResult2 r) {
	    	q.queue.add(321);
	    	Integer result = q.queue.poll();
	    	r.r2 = (result == null ? -1 : result);
	    }
	}
	
	@JCStressTest
    @Outcome(id = "1", expect = Expect.ACCEPTABLE, desc = "queue is not empty")
    @Outcome(id = "0", expect = Expect.FORBIDDEN, desc = "empty queue is race")
	public static class IsEmptyTest {

	    @Actor
	    public void actor1(QueueState q) {
	    	q.queue.add(123);
	    }

	    @Actor
	    public void actor2(QueueState q) {
	    	q.queue.add(321);
	    }
	    
        @Arbiter
        public void arbiter(QueueState q, IntResult1 result) {
            result.r1 = (!q.queue.isEmpty() ? 1 : 0);
        }

	}

	@JCStressTest
	@Outcome(id = "2, 123, 123", expect = Expect.ACCEPTABLE, desc = "returns 2, 123 and 321")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
	public static class PeekSizeTest {

	    @Actor
	    public void actor1(QueueState q) {
	    	q.queue.add(123);
	    }

	    @Actor
	    public void actor2(QueueState q) {
	    	q.queue.add(321);
	    }
	    
        @Arbiter
        public void arbiter(QueueState q, IntResult3 r) {
        	Integer result2 = q.queue.peek();
        	Integer result3 = q.queue.peek();
        	int size = q.queue.size();
        	r.r1 = size;
            r.r2 = (result2 == null ? -1 : result2);
            r.r3 = (result3 == null ? -1 : result3);
        }

	}
}

