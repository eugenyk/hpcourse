package ru.spbau.mit;

import org.junit.Assert;
import org.junit.Test;

import java.lang.reflect.Array;
import java.util.*;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.LongAccumulator;
import java.util.concurrent.atomic.LongAdder;
import java.util.stream.Collectors;

public class PriorityQueueLockFreeImplTest {
    private ThreadLocalRandom random = ThreadLocalRandom.current();

    private Queue<Integer> makeLockFreeQueue(boolean debug) {
        return new PriorityQueueLockFreeImpl<>(debug);
    }


    @Test
    public void pollOfferConcurrency() throws InterruptedException, ExecutionException {
        for (int testIteration = 0; testIteration < 10; testIteration++) {
            final int amount = random.nextInt(2016, 50_000);
            final int[] storage = random.ints(amount).toArray();

            final AtomicInteger storageTailSize = new AtomicInteger(amount);
            final AtomicBoolean canStart = new AtomicBoolean(false);
            final Queue<Integer> lf_pq = makeLockFreeQueue(false);

            Runnable producer = () -> {
                while (!canStart.get()) {
                    Thread.yield();
                }
                while (true) {
                    int index = storageTailSize.get();
                    if (index <= 0) {
                        break;
                    }
                    if (storageTailSize.compareAndSet(index, index - 1)) {
                        lf_pq.offer(storage[index - 1]);
                    }
                }
            };

            Callable<Map<Integer, Long>> consumer = () -> {
                while (!canStart.get()) {
                    Thread.yield();
                }
                Map<Integer, Long> multiset = new HashMap<>();
                while (true) {
                    Integer value = lf_pq.poll();
                    if (value != null) {
                        Long counter = Optional.ofNullable(multiset.get(value)).orElse(0L);
                        multiset.put(value, counter + 1);
                    } else if (storageTailSize.get() <= 0) {
                        Thread.sleep(200, 100);
                        if (lf_pq.size() == 0) {
                            break;
                        }
                    }
                }
                return multiset;
            };

            int amountTreads = random.nextInt(11, 42);
            ExecutorService pool = Executors.newFixedThreadPool(amountTreads);
            ArrayList<Future<Map<Integer, Long>>> results = new ArrayList<>();
            for (int i = 0; i < amountTreads; i++) {
                pool.submit(producer);
                results.add(pool.submit(consumer));
            }

            canStart.set(true);
            Thread.sleep(200, 11);

            TreeMap<Integer, Long> global_multiset = new TreeMap<>();
            for (int i = 0; i < amountTreads; i++) {
                Map<Integer, Long> local_multiset = results.get(i).get();
                for (Map.Entry<Integer, Long> entry : local_multiset.entrySet()) {
                    long counter = Optional.ofNullable(global_multiset.get(entry.getKey())).orElse(0L);
                    global_multiset.put(entry.getKey(), counter + entry.getValue());
                }
            }

            TreeMap<Integer, Long> expected_multiset = Arrays.stream(storage)
                    .boxed()
                    .collect(Collectors.groupingBy(
                            i -> i,
                            TreeMap::new,
                            Collectors.counting()
                    ));

            Assert.assertEquals(0, lf_pq.size());
            Assert.assertEquals(expected_multiset.size(), global_multiset.size());
            for (Map.Entry<Integer, Long> entry : expected_multiset.entrySet()) {
                Assert.assertEquals(entry.getValue(), global_multiset.get(entry.getKey()));
            }
        }
    }


    @Test
    public void pollOfferHalfDuplex() throws ExecutionException, InterruptedException {
        int amount = random.nextInt(50_000, 100_000);
        int[] storage = random.ints(amount).toArray();

        Queue<Integer> lf_pq = makeLockFreeQueue(false);
        Arrays.stream(storage)
                .parallel()
                .forEach(lf_pq::offer);

        Callable<Boolean> callable = () -> {
            int prevValue = Integer.MIN_VALUE;
            Integer localValue;
            boolean good = true;
            while (!Thread.currentThread().isInterrupted()) {
                localValue = lf_pq.poll();
                if (localValue == null) {
                    break;
                }
                if (prevValue > localValue) {
                    good = false;
                    break;
                }
                prevValue = localValue;
            }
            return good;
        };

        int amountTreads = random.nextInt(11, 42);
        ExecutorService pool = Executors.newFixedThreadPool(amountTreads);
        ArrayList<Future<Boolean>> results = new ArrayList<>();
        for (int i = 0; i < amountTreads; i++) {
            results.add(pool.submit(callable));
        }
        Thread.sleep(200, 11);

        for (int i = 0; i < amountTreads; i++) {
            Assert.assertTrue(results.get(i).get());
        }
    }


    @Test
    public void pollAccumulation() {
        int amount = random.nextInt(2016, 50_000);
        int[] storage = random.ints(amount).toArray();

        Map<Integer, Long> expected_multiset = Arrays.stream(storage).boxed()
                .collect(Collectors.groupingBy(
                        i -> i,
                        Collectors.counting()
                ));

        Queue<Integer> lf_pq = makeLockFreeQueue(false);
        Arrays.stream(storage)
                .parallel()
                .forEach(lf_pq::offer);

        Map<Integer, Long> actual_multiset = Arrays.stream(storage)
                .parallel()
                .mapToObj(i -> lf_pq.poll())
                .collect(Collectors.groupingBy(
                        i -> i,
                        Collectors.counting()
                ));

        Assert.assertEquals(expected_multiset, actual_multiset);
    }


    @Test
    public void offer() {
        Queue<Integer> lf_pq = makeLockFreeQueue(false);
        Queue<Integer> pq = new PriorityQueue<>();

        int amount = random.nextInt(2016, 50_000);
        int[] storage = random.ints(amount).toArray();

        Arrays.stream(storage)
                .sequential()
                .forEach(pq::offer);

        Arrays.stream(storage)
                .parallel()
                .forEach(lf_pq::offer);

        for (int i = 0; i < amount; i++) {
            Integer expected_value = pq.poll();
            Integer actual_value = lf_pq.poll();
            Assert.assertEquals(expected_value, actual_value);
        }
    }

    @Test
    public void size() {
        Queue<Integer> pq = makeLockFreeQueue(false);
        int amount = random.nextInt(2016, 50_000);
        random.ints(amount)
                .parallel()
                .forEach(pq::offer);
        Assert.assertEquals("size must be the same", amount, pq.size());
    }
}
