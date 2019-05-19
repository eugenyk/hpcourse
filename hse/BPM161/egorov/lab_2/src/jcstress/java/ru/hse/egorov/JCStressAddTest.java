package ru.hse.egorov;

import org.apache.commons.collections4.IteratorUtils;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;
import org.openjdk.jcstress.util.StringUtils;

import java.util.stream.Collectors;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


@JCStressTest
@Outcome(id = "1, 2", expect = ACCEPTABLE,  desc = "Both updates.")
@Outcome(id = "1", expect = FORBIDDEN,  desc = "One lost.")
@Outcome(id = "2", expect = FORBIDDEN,  desc = "One lost.")
@Outcome(id = "", expect = FORBIDDEN,  desc = "All lost.")
@State
public class JCStressAddTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
    }

    @Actor
    public void actor2() {
        set.add(2);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = StringUtils.join(IteratorUtils.toList(set.iterator()).stream().map(Object::toString).collect(Collectors.toList()), ", ");
    }
}