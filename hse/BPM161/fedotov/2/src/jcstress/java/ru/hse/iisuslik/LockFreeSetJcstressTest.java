package ru.hse.iisuslik;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZ_Result;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

public class LockFreeSetJcstressTest {

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

    @JCStressTest
    @Outcome(id = "true, false, false, true", expect = ACCEPTABLE, desc = "First add succeeded, remove didn't.")
    @Outcome(id = "true, false, true, false", expect = ACCEPTABLE, desc = "First add succeeded, so did remove.")
    @Outcome(id = "false, true, false, true", expect = ACCEPTABLE, desc = "Second add succeeded, remove didn't.")
    @Outcome(id = "false, true, true, false", expect = ACCEPTABLE, desc = "Second add succeeded, so did remove.")
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All queries succeeded.")
    @State
    public static class SameValueAddTest {
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
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "Successfully removed everything.")
    @State
    public static class RemoveTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        RemoveTest() {
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
    @Outcome(id = "true", expect = ACCEPTABLE, desc = "All values successfully added.")
    @State
    public static class AddTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            int i = 0;
            set.add(i);
            set.add(i + 4);
            set.add(i + 8);
        }

        @Actor
        public void actor2() {
            int i = 1;
            set.add(i);
            set.add(i + 4);
            set.add(i + 8);
        }

        @Actor
        public void actor3() {
            int i = 2;
            set.add(i);
            set.add(i + 4);
            set.add(i + 8);
        }

        @Actor
        public void actor4() {
            int i = 3;
            set.add(i);
            set.add(i + 4);
            set.add(i + 8);
        }

        @Arbiter
        public void arbiter(Z_Result r) {
            boolean isOk = true;
            for (int i = 0; i < 12; i++) {
                isOk &= set.contains(i);
            }
            r.r1 = isOk;

        }
    }

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All values successfully added.")
    @Outcome(id = "true, true, false, true", expect = ACCEPTABLE, desc = "All values successfully added except 3")
    @Outcome(id = "true, false, false, true", expect = ACCEPTABLE, desc = "All values successfully added except 2")
    @Outcome(id = "false, false, false, true", expect = ACCEPTABLE, desc = "No values added.")
    @State
    public static class IteratorTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1(ZZZZ_Result r) {
            set.add(1);
            set.add(2);
            set.add(3);
        }


        @Actor
        public void actor2(ZZZZ_Result r) {
            set.iterator().forEachRemaining(i -> {
                if (i == 1) {
                    r.r1 = true;
                } else if (i == 2) {
                    r.r2 = true;
                } else if (i == 3) {
                    r.r3 = true;

                }
            });
        }

        @Arbiter
        public void arbiter(ZZZZ_Result r) {
            r.r4 = set.contains(1) && set.contains(2) && set.contains(3);
        }
    }
}
