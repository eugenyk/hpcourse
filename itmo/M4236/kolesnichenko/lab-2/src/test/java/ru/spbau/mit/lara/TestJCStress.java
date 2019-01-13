package ru.spbau.mit.lara;


import org.graalvm.compiler.core.common.util.IntList;
import org.junit.Assert;
import org.junit.Test;

import java.util.*;
import java.util.PriorityQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

public class TestJCStress {
    @Test
    public void singleThreadInsert() {
        LockFreeSortedList<Integer> pq = new LockFreeSortedList<>();
        assertEquals(0, pq.size());
        pq.offer(1);
        assertEquals(1, pq.size());
        assertEquals(1, pq.peek().intValue());
        pq.offer(2); // insert higher priority
        assertEquals(2, pq.size());
        assertEquals(1, pq.peek().intValue());
        pq.offer(-1); // insert lower priority
        assertEquals(3, pq.size());
        assertEquals(-1, pq.peek().intValue());
        pq.offer(-1); // insert same priority
        assertEquals(4, pq.size());
        assertEquals(-1, pq.peek().intValue());
    }
    @Test
    public void singleThreadInsertRemove() {
        LockFreeSortedList<Integer> q = new LockFreeSortedList<>();
        q.offer(1);
        q.offer(2);
        q.offer(-1);
        q.offer(1);
        assertEquals(4, q.size());
        int i = q.poll();
        assertEquals(-1, i);
        assertEquals(3, q.size());
        i = q.poll();
        assertEquals(1, i);
        assertEquals(2, q.size());
        i = q.poll();
        assertEquals(1, i);
        assertEquals(1, q.size());
        i = q.poll();
        assertEquals(2, i);
        assertEquals(0, q.size());
        q.offer(1);
        assertEquals(1, q.size());
        i = q.poll();
        assertEquals(1, i);
        assertEquals(0, q.size());
        assertNull(null, q.poll());
    }

    @Test
    public void parallelInsert() {
        final int n = 20000;
        Queue<Integer> pq = new LockFreeSortedList<>();
        ThreadLocalRandom valuesSupplier = ThreadLocalRandom.current();
        List<Integer> values = valuesSupplier.ints(n).boxed().collect(Collectors.toList());
        values.parallelStream().forEach(pq::offer);
        assertEquals(n, pq.size());
        int prev = Integer.MIN_VALUE;
        for (int i = 0; i < n; i++) {
            int polled = pq.poll();
            assertTrue(prev <= polled);
            prev = polled;
        }
    }

    @Test
    public void parallelInsertRemove() {
        final int n = 20000;
        Queue<Integer> pq = new LockFreeSortedList<>();
        ThreadLocalRandom valuesSupplier = ThreadLocalRandom.current();
        List<Integer> values = valuesSupplier.ints(n).boxed().collect(Collectors.toList());
        values.parallelStream().forEach(pq::offer);
        Map<Long, List<Integer>> results = new HashMap<>();
        values.parallelStream().forEach(i -> {
            int res = pq.poll();
            Long threadId = Thread.currentThread().getId();
            if (!results.containsKey(threadId)) {
                results.put(threadId, new ArrayList<>());
            }
            results.get(threadId).add(res);
        });
        results.forEach((id, list) -> {
            int prev = list.get(0);
            for (int i : list) {
                assertTrue(prev <= i);
                prev = i;
            }
        });
    }
}
