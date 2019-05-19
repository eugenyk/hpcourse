package ru.hse.nikiforovskaya.set;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.FFF_Result;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import java.util.ArrayList;
import java.util.List;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


public class LockFreeSetStressTest {
    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All values successfully added.")
    @State
    public static class ConcurrentRaceTest {
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
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "Order: 1, 3, 2 or 2, 3, 1")
    @Outcome(id = "true, false, true, false", expect = ACCEPTABLE, desc = "Order: 1, 2, 3")
    @Outcome(id = "false, true, true, false", expect = ACCEPTABLE, desc = "Order: 2, 1, 3")
    @Outcome(id = "true, false, false, true", expect = ACCEPTABLE, desc = "Order: 3, 1, 2")
    @Outcome(id = "false, true, false, true", expect = ACCEPTABLE, desc = "Order: 3, 2, 1")
    @State
    public static class SameValueTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1(ZZZZ_Result r) {
            r.r1 = set.add(1);
        }

        @Actor
        public void actor2(ZZZZ_Result r) {
            r.r2 = set.add(1);
        }

        @Actor
        public void actor3(ZZZZ_Result r) {
            r.r3 = set.remove(1);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result r) {
            r.r4 = set.contains(1);
        }
    }

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All operations were successful")
    @State
    public static class RemovableTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        public RemovableTest() {
            set.add(1);
            set.add(2);
            set.add(3);
        }

        @Actor
        public void actor1(ZZZZ_Result r) {
            r.r1 = set.remove(1);
        }

        @Actor
        public void actor2(ZZZZ_Result r) {
            r.r2 = set.remove(2);
        }

        @Actor
        public void actor3(ZZZZ_Result r) {
            r.r3 = set.remove(3);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result r) {
            r.r4 = set.isEmpty();
        }
    }


    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All operations were successful")
    @State
    public static class ManyAddsIteratorTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            set.add(1);
            set.add(2);
        }

        @Actor
        public void actor2() {
            set.add(3);
            set.add(4);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result r) {
            List<Integer> list = new ArrayList<>();
            set.iterator().forEachRemaining(list::add);
            r.r1 = list.size() == 4;
            r.r2 = list.contains(1) && list.contains(2) && list.contains(3) && list.contains(4);
            r.r3 = set.contains(1) && set.contains(2) && set.contains(3) && set.contains(4);
            r.r4 = list.indexOf(1) < list.indexOf(2) && list.indexOf(3) < list.indexOf(4);
        }
    }
}