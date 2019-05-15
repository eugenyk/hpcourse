package ru.hse.spb.hpcourse.set;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


public class LockFreeSetStressTest {
    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All values successfully added.")
    @State
    public static class ConcurrentAddTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            set.add(1);
        }

        @Actor
        public void actor2() {
            set.add(2);
        }

        @Actor
        public void actor3() {
            set.add(3);
        }

        @Actor
        public void actor4() {
            set.add(4);
        }


        @Arbiter
        public void arbiter(ZZZZ_Result r) {
            r.r1 = set.contains(1);
            r.r2 = set.contains(2);
            r.r3 = set.contains(3);
            r.r4 = set.contains(4);
        }
    }

    @JCStressTest
    @Outcome(id = "true, true", expect = ACCEPTABLE, desc = "All values successfully added before queries.")
    @Outcome(id = "true, false", expect = ACCEPTABLE, desc = "First added after query for second.")
    @Outcome(id = "false, false", expect = ACCEPTABLE, desc = "None added before queries")
    @Outcome(id = "false, true", expect = FORBIDDEN, desc = "Already have seen second add, but not the first add.")
    @State
    public static class ConsistencyTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            set.add(1);
            set.add(2);
        }

        @Actor
        public void actor2(ZZ_Result r) {
            r.r2 = set.contains(2);
            r.r1 = set.contains(1);
        }
    }
}