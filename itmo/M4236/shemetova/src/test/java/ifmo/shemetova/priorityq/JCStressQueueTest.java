package ifmo.shemetova.priorityq;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

import java.util.Queue;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;


public class JCStressQueueTest {
    @JCStressTest
    @Outcome(id = "50", expect = ACCEPTABLE, desc = "Size of queue is correct")
    public static class AddManyTest {
        @State
        public static class QState {
            Queue<Integer> queue = new HPPriorityQueue<>();
        }

        @Actor
        public void actor1(QState state) {
            for (int i = 0; i < 10; i++) {
                state.queue.offer(i);

            }
        }

        @Actor
        public void actor2(QState state) {
            for (int i = 0; i < 10; i++) {
                state.queue.offer(i);

            }
        }

        @Actor
        public void actor3(QState state) {
            for (int i = 0; i < 10; i++) {
                state.queue.offer(i);

            }
        }

        @Actor
        public void actor4(QState state) {
            for (int i = 0; i < 10; i++) {
                state.queue.offer(i);

            }
        }

        @Actor
        public void actor5(QState state) {
            for (int i = 0; i < 10; i++) {
                state.queue.offer(i);

            }
        }

        @Arbiter
        public void arbiter(QState state, I_Result res) {
            res.r1 = state.queue.size();
        }
    }

    @JCStressTest
    @Outcome(id = "0", expect = ACCEPTABLE, desc = "Nothing to add")
    @Outcome(id = "4", expect = ACCEPTABLE, desc = "4")
    @Outcome(id = "3", expect = ACCEPTABLE, desc = "3")
    @Outcome(id = "2", expect = ACCEPTABLE, desc = "2")
    @Outcome(id = "1", expect = ACCEPTABLE, desc = "1")
    public static class PeekTest {
        @State
        public static class QState {
            Queue<Integer> queue = new HPPriorityQueue<>();
        }

        @Actor
        public void actor1(QState state) {
            state.queue.offer(4);
            state.queue.offer(3);
            state.queue.offer(2);
            state.queue.offer(1);
        }

        @Actor
        public void actor2(QState state, I_Result res) {
            Integer top = state.queue.peek();
            if (top != null) {  res.r1 = top;}
            else { res.r1 = 0; }
        }

    }

}
