package ru.sofysmo.priorityqueue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;
import org.openjdk.jcstress.infra.results.I_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class LockFreePriorityQueueStressTest {

    @State
    public static class QueueState {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<Integer>();
    }

    @JCStressTest
    @Outcome(id = "2, 1, 12", expect = Expect.ACCEPTABLE, desc = "returns 2, 1 and 12")
    @Outcome(id = "1, 1, -1", expect = Expect.ACCEPTABLE, desc = "returns 1, 1 and -1")
    @Outcome(id = "1, 12, -1", expect = Expect.ACCEPTABLE, desc = "returns 1, 12 and -1")
    @Outcome(id = "0, -1, -1", expect = Expect.ACCEPTABLE, desc = "returns 0, -1 and -1")
    @Outcome(id = "2, 1, -1", expect = Expect.ACCEPTABLE_INTERESTING, desc = "One update lost: atomicity failure.")
    @Outcome(id = "2, 12, -1", expect = Expect.ACCEPTABLE_INTERESTING, desc = "One update lost: atomicity failure.")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class OfferTest1 {

        @Actor
        public void actor1(QueueState q) {
            q.queue.offer(1);
        }

        @Actor
        public void actor2(QueueState q) {
            q.queue.offer(12);
        }

        @Arbiter
        public void arbiter(QueueState q, III_Result r) {
            int size = q.queue.size();
            Integer result2 = q.queue.poll();
            Integer result3 = q.queue.poll();
            r.r1 = size;
            r.r2 = (result2 == null ? -1 : result2);
            r.r3 = (result3 == null ? -1 : result3);
        }

    }

    @JCStressTest
    @Outcome(id = "-1, -1, -1", expect = ACCEPTABLE)
    @Outcome(id = "1, -1, -1", expect = ACCEPTABLE)
    @Outcome(id = "1, 2, -1", expect = ACCEPTABLE)
    @Outcome(id = "1, 2, 3", expect = ACCEPTABLE)
    @Outcome(id = "2, -1, -1", expect = ACCEPTABLE)
    @Outcome(id = "2, 3, -1", expect = ACCEPTABLE)
    @Outcome(id = "3, -1, -1", expect = ACCEPTABLE)
    public static class OfferIsConsistent {
        @State
        public static class QueueState {
            final PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
        }

        @Actor
        public void actor1(QueueState state) {
            state.queue.offer(1);
            state.queue.offer(3);
            state.queue.offer(2);
        }

        @Actor
        public void actor2(QueueState state) {
            state.queue.poll();
        }

        @Arbiter
        public void arbiter(QueueState state, III_Result res) {
            Integer r1 = state.queue.poll();
            Integer r2 = state.queue.poll();
            Integer r3 = state.queue.poll();

            res.r1 = r1 != null ? r1 : -1;
            res.r2 = r2 != null ? r2 : -1;
            res.r3 = r3 != null ? r3 : -1;
        }
    }

}
