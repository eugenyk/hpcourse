package ru.hse.spb.solikov;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class LockFreeSetJcstress {

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "Actors first, arbiter after")
    @Outcome(id = "true, true, true, false", expect = ACCEPTABLE, desc = "Actor1, arbiter, actor2")
    @Outcome(id = "true, true, false, true", expect = ACCEPTABLE, desc = "Actor2, arbiter, actor1")
    @Outcome(id = "true, true, false, false", expect = ACCEPTABLE, desc = "Arbiter, actors")
    @State
    public static class SetAdd {

        private LockFreeSet<Integer> set = new LockFreeSetWithSnapshots<>();

        @Actor
        public void actor1(ZZZZ_Result res) {
            res.r1 = set.add(10);
        }

        @Actor
        public void actor2(ZZZZ_Result res) {
            res.r2 = set.add(20);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result res) {
            res.r3 = set.contains(10);
            res.r4 = set.contains(20);
        }
    }

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "Good")
    @Outcome(id = "true, true, true, false", expect = ACCEPTABLE, desc = "Not finished")
    @State
    public static class SetRemove {

        private LockFreeSet<Integer> set = new LockFreeSetWithSnapshots<>();

        public SetRemove() {
            set.add(1);
            set.add(2);
            set.add(3);
        }

        @Actor
        public void actor1(ZZZZ_Result res) {
            res.r1 = set.remove(1);
        }

        @Actor
        public void actor2(ZZZZ_Result res) {
            res.r2 = set.remove(2);
        }

        @Actor
        public void actor3(ZZZZ_Result res) {
            res.r3 = set.remove(3);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result res) {
            res.r4 = set.isEmpty();
        }
    }
}
