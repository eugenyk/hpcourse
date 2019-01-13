package org.anstreth.queue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;
import org.openjdk.jcstress.infra.results.I_Result;

import java.util.Queue;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class JCStressQueueTestSuite {

    @JCStressTest
    @Outcome(id = "100", expect = ACCEPTABLE, desc = "size of map = 100")
    public static class SizeIsConsistentTest {
        @State
        public static class QueueState {
            final Queue<Integer> queue = new ConcurrentPriorityQueue<>();
        }

        @Actor
        public void actor1(QueueState state) {
            for (int i = 0; i < 50; i++) {
                state.queue.offer(i);
            }
        }

        @Actor
        public void actor2(QueueState state) {
            for (int i = 0; i < 50; i++) {
                state.queue.offer(i);
            }
        }

        @Arbiter
        public void arbiter(QueueState state, I_Result res) {
            res.r1 = state.queue.size();
        }

        @JCStressTest
        // third position
        @Outcome(id = "-1, -1, -1", expect = ACCEPTABLE)
        @Outcome(id = "-1, -1, 1", expect = ACCEPTABLE)
        @Outcome(id = "-1, -1, 3", expect = ACCEPTABLE)

        // second position
        @Outcome(id = "-1, 1, -1", expect = ACCEPTABLE)
        @Outcome(id = "-1, 1, 3", expect = ACCEPTABLE)

        @Outcome(id = "-1, 3, 1", expect = ACCEPTABLE)
        @Outcome(id = "-1, 3, 2", expect = ACCEPTABLE)

        // first position
        @Outcome(id = "1, -1, -1", expect = ACCEPTABLE)
        @Outcome(id = "1, -1, 3", expect = ACCEPTABLE)
        @Outcome(id = "1, 3, -1", expect = ACCEPTABLE)
        @Outcome(id = "1, 3, 2", expect = ACCEPTABLE)

        @Outcome(id = "3, 2, 1", expect = ACCEPTABLE)
        @Outcome(id = "3, 1, -1", expect = ACCEPTABLE)
        @Outcome(id = "3, 1, 2", expect = ACCEPTABLE)
        public static class OfferIsConsistent {
            @State
            public static class QueueState {
                final Queue<Integer> queue = new ConcurrentPriorityQueue<>();
            }

            @Actor
            public void actor1(QueueState state) {
                state.queue.offer(1);
                state.queue.offer(3);
                state.queue.offer(2);
            }

            @Actor
            public void actor3(QueueState state, III_Result res) {
                Integer r1 = state.queue.poll();
                Integer r2 = state.queue.poll();
                Integer r3 = state.queue.poll();

                res.r1 = r1 != null ? r1 : -1;
                res.r2 = r2 != null ? r2 : -1;
                res.r3 = r3 != null ? r3 : -1;
            }
        }
    }

}