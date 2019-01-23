package jcstree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;
import priorityQueue.ConcurrentPriorityQueueImpl;

import java.util.Queue;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class ConcurrentPriorityQueueImplTestJCStress {

    @JCStressTest
    @Outcome(id = "150", expect = ACCEPTABLE)
    public static class SizeIsCorrectTest {
        @State
        public static class QueueState {
            final Queue<Integer> queue = new ConcurrentPriorityQueueImpl<>();
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

        @Actor
        public void actor3(QueueState state) {
            for (int i = 0; i < 50; i++) {
                state.queue.offer(i);
            }
        }

        @Arbiter
        public void arbiter(QueueState state, I_Result result) {
            result.r1 = state.queue.size();
        }
    }

    @JCStressTest
    @Outcome(id = "0", expect = ACCEPTABLE)
    @Outcome(id = "1", expect = ACCEPTABLE)
    @Outcome(id = "2", expect = ACCEPTABLE)
    @Outcome(id = "3", expect = ACCEPTABLE)
    public static class OfferAndPeekTest {
        @State
        public static class QueueState {
            final Queue<Integer> queue = new ConcurrentPriorityQueueImpl<>();
        }

        @Actor
        public void actor1(QueueState state) {
            state.queue.offer(3);
            state.queue.offer(2);
            state.queue.offer(1);
        }

        @Actor
        public void actor2(QueueState state, I_Result result) {
            Integer r1 = state.queue.peek();
            result.r1 = (r1 != null) ? r1 : 0;
        }

    }

}