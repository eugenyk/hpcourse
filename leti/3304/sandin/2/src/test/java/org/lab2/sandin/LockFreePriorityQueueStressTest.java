package org.lab2.sandin;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;
import org.openjdk.jcstress.infra.results.I_Result;
import org.lab2.sandin.*;
import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class LockFreePriorityQueueStressTest {
    @State
    public static class QueueState {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    }

    @JCStressTest
    @Outcome(id = "1, 2", expect = Expect.ACCEPTABLE)
    @Outcome(id = "2, 1", expect = Expect.ACCEPTABLE)
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity.")
    public static class EnqOfferPeekTest {

        @Actor
        public void actor1(QueueState q, II_Result r) {
            q.queue.offer(1);
            Integer result = q.queue.peek();
            r.r1 = (result == null ? -1 : result);
        }

        @Actor
        public void actor2(QueueState q, II_Result r) {
            q.queue.offer(2);
            Integer result = q.queue.peek();
            r.r2 = (result == null ? -1 : result);
        }
    }
    
    @JCStressTest
    @Outcome(id = "2", expect = Expect.ACCEPTABLE)
    @Outcome(id = "1", expect = Expect.FORBIDDEN, desc = "size of map = 1 is race")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class EnqSizeTest {

        @Actor
        public void actor1(QueueState q) {
            q.queue.offer(1);
        }

        @Actor
        public void actor2(QueueState q) {
            q.queue.offer(2);
        }

        @Arbiter
        public void arbiter(QueueState q,  I_Result r) {
            r.r1 = q.queue.size();
        }

    }

    @JCStressTest
    @Outcome(id = "1, 2", expect = ACCEPTABLE)
    @Outcome(id = "1, 0", expect = ACCEPTABLE)
    @Outcome(id = "2, 0", expect = ACCEPTABLE)
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class EnqOfferPollTest {

        @State
        public static class QueueState {
            final PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
        }

        @Actor
        public void actor1(QueueState state) {
            state.queue.offer(2);
            state.queue.offer(1);
        }

        @Actor
        public void actor2(QueueState state) {
            state.queue.poll();
        }

        @Arbiter
        public void arbiter(QueueState state, II_Result res) {
            Integer r1 = state.queue.poll();
            Integer r2 = state.queue.poll();

            res.r1 = r1 != null ? r1 : 0;
            res.r2 = r2 != null ? r2 : 0;
        }
    }
}