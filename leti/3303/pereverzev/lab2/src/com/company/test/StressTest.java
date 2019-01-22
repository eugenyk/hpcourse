package com.company.test;

import com.company.*;
import org.openjdk.jcstress.infra.results.III_Result;
import org.openjdk.jcstress.infra.results.II_Result;
import org.openjdk.jcstress.infra.results.I_Result;
import org.openjdk.jcstress.annotations.*;

public class StressTest {
    @State
    public static class QueueState {
        final LockFreePriorityLFQueue<Integer> queue = new LockFreePriorityLFQueue<>();
    }

    @JCStressTest
    @Outcome(id = "2, 123, 321", expect = Expect.ACCEPTABLE, desc = "returns 2, 123 and 321")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class EnqSizeTest {

        @Actor
        public void actor1(QueueState q) {
            q.queue.offer(123);
        }

        @Actor
        public void actor2(QueueState q) {
            q.queue.offer(321);
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
    @Outcome(id = "123, 321", expect = Expect.ACCEPTABLE, desc = "returns 123 and 321")
    @Outcome(id = "321, 123", expect = Expect.ACCEPTABLE, desc = "returns 321 and 123")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class EnqDeqTest {

        @Actor
        public void actor1(QueueState q, II_Result r) {
            q.queue.offer(123);
            Integer result = q.queue.poll();
            r.r1 = (result == null ? -1 : result);
        }

        @Actor
        public void actor2(QueueState q, II_Result r) {
            q.queue.offer(321);
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
            q.queue.offer(123);
        }

        @Actor
        public void actor2(QueueState q) {
            q.queue.offer(321);
        }

        @Arbiter
        public void arbiter(QueueState q, I_Result result) {
            result.r1 = (!q.queue.isEmpty() ? 1 : 0);
        }

    }

    @JCStressTest
    @Outcome(id = "2, 123, 123", expect = Expect.ACCEPTABLE, desc = "returns 2, 123 and 321")
    @Outcome(expect = Expect.FORBIDDEN, desc = "Case violating atomicity")
    public static class PeekSizeTest {

        @Actor
        public void actor1(QueueState q) {
            q.queue.offer(123);
        }

        @Actor
        public void actor2(QueueState q) {
            q.queue.offer(321);
        }

        @Arbiter
        public void arbiter(QueueState q, III_Result r) {
            Integer result2 = q.queue.peek();
            Integer result3 = q.queue.peek();
            int size = q.queue.size();
            r.r1 = size;
            r.r2 = (result2 == null ? -1 : result2);
            r.r3 = (result3 == null ? -1 : result3);
        }

    }
}