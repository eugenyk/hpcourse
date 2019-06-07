package ru.spbau.gbarto;

import org.apache.commons.collections4.IteratorUtils;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;
import org.openjdk.jcstress.util.StringUtils;

import java.util.stream.Collectors;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "8, 800", expect = ACCEPTABLE)
@State
public class JCStressAdd {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(8);
    }

    @Actor
    public void actor2() {
        set.add(800);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = StringUtils.join(
                IteratorUtils.toList(set.iterator()).stream()
                        .map(Object::toString)
                        .collect(Collectors.toList()),
                ", "
        );
    }
}