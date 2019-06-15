package me.eranik.lockfree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

@JCStressTest
@Description("Check that iterator works correctly")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Iterator is correct")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Iterator is incorrect")
@State
public class LockFreeIterator {
    private Set<Integer> set = new LockFreeSet<>();
    private ArrayList<TreeSet<Integer>> producerResults = new ArrayList<>();
    private ArrayList<TreeSet<Integer>> consumerResults = new ArrayList<>();
    private AtomicInteger producerIter = new AtomicInteger(0);
    private ArrayList<Integer> iters = new ArrayList<>();

    private final int PROD_ITER = 2500;
    private final int CONS_ITER = 25;
    private final int MAX_VAL = 10;

    @Actor
    public void producer() {
        TreeSet<Integer> tree = new TreeSet<>();
        Random random = new Random(13);
        producerResults.add((TreeSet<Integer>) tree.clone());
        for (int i = 0; i < PROD_ITER; ++i) {
            producerIter.incrementAndGet();
            int value = random.nextInt(MAX_VAL);
            if (random.nextBoolean()) {
                if (set.add(value)) {
                    tree.add(value);
                }
            } else {
                if (set.remove(value)) {
                    tree.remove(value);
                }
            }
            producerResults.add((TreeSet<Integer>) tree.clone());
        }
    }

    @Actor
    public void consumer() {
        for (int i = 0; i < CONS_ITER; ++i) {
            TreeSet<Integer> tree = new TreeSet<>();
            Iterator<Integer> iter = set.iterator();
            while (iter.hasNext()) {
                tree.add(iter.next());
            }
            consumerResults.add(tree);
            iters.add(producerIter.get());
        }
    }

    @Arbiter
    public void checker(L_Result result) {
        boolean correct = true;

        for (int i = 0, j = 0; i < CONS_ITER; ++i) {
            int iter = iters.get(i);
            TreeSet<Integer> consTree = consumerResults.get(i);

            boolean found = false;
            while (j <= iter) {
                TreeSet<Integer> prodTree = producerResults.get(j);
                if (prodTree.size() == consTree.size() && prodTree.containsAll(consTree)) {
                    found = true;
                    break;
                }
                j++;
            }

            correct &= found;
        }

        result.r1 = correct;
    }
}
