package ru.hse.spb.kazakov

import org.openjdk.jcstress.annotations.Expect
import org.openjdk.jcstress.annotations.JCStressTest
import org.openjdk.jcstress.annotations.Outcome
import org.openjdk.jcstress.annotations.State
import org.openjdk.jcstress.annotations.Arbiter
import org.openjdk.jcstress.annotations.Actor
import org.openjdk.jcstress.infra.results.Z_Result


@JCStressTest
@Outcome.Outcomes(
    Outcome(id = ["true"], expect = Expect.ACCEPTABLE, desc = "All elements are added."),
    Outcome(id = ["false"], expect = Expect.FORBIDDEN, desc = "Some elements are missing.")
)
@State
class LockFreeSetAddTest {
    private val set = LockFreeSet<Int>()

    @Actor
    fun add1() {
        for (i in 0..20) {
            set.add(i)
        }
    }

    @Actor
    fun add2() {
        for (i in 21..40) {
            set.add(i)
        }
    }

    @Arbiter
    fun arbiter(r: Z_Result) {
        var result = true
        for (i in 0..40) {
            result = result && set.contains(i)
        }
        r.r1 = result
    }
}

@JCStressTest
@Outcome.Outcomes(
    Outcome(id = ["true"], expect = Expect.ACCEPTABLE, desc = "All elements are removed."),
    Outcome(id = ["false"], expect = Expect.FORBIDDEN, desc = "Some elements are still in the set.")
)
@State
class LockFreeSetRemoveTest {
    private val set = LockFreeSet<Int>()

    init {
        for (i in 0..30) {
            set.add(i)
        }
    }

    @Actor
    fun remove1() {
        for (i in 0..20) {
            set.remove(i)
        }
    }

    @Actor
    fun remove2() {
        for (i in 10..30) {
            set.remove(i)
        }
    }

    @Arbiter
    fun arbiter(r: Z_Result) {
        r.r1 = set.isEmpty()
    }
}