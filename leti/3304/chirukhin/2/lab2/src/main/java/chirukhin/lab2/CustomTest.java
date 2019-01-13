package chirukhin.lab2;

import chirukhin.lab2.priorityqueue.LockFreePriorityQueue;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.*;
import org.junit.Test;

public class CustomTest {
    private static int NUM_THREADS = 10;

    @Test
    public void MultipleProducersSingleConsumer_PollRightPriorityOrder()   {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

        ExecutorService threadPool = Executors.newFixedThreadPool(NUM_THREADS);

        final CountDownLatch latch = new CountDownLatch(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; i++) {
            threadPool.submit(() -> {
                for (int i1 = 0; i1 < 10000; i1++) {
                    int nextInt = ThreadLocalRandom.current().nextInt(-1000, 2000);
                    queue.add(nextInt);
                }
                latch.countDown();
            });
        }

        try {
            latch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        int prev = Integer.MIN_VALUE;
        int curr;
        while (!queue.isEmpty()) {
            curr = queue.poll();
            assert(curr >= prev);
            prev = curr;
        }
    }

    @Test
    public void MultipleProducersMultipleConsumers_SumAddEqualsSumPoll()   {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

        ExecutorService threadPool = Executors.newFixedThreadPool(NUM_THREADS);

        // add
        final CountDownLatch addLatch = new CountDownLatch(NUM_THREADS);
        List<Future<Integer>> futuresAdded = new ArrayList<>();
        for (int i = 0; i < NUM_THREADS; i++) {
            Future<Integer> result = threadPool.submit(() -> {
                int sum = 0;
                for (int i1 = 0; i1 < 10000; i1++) {
                    int nextInt = ThreadLocalRandom.current().nextInt(-1000, 2000);
                    queue.add(nextInt);
                    sum += nextInt;
                }
                addLatch.countDown();
                return sum;
            });
            futuresAdded.add(result);
        }

        try {
            addLatch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // poll
        final CountDownLatch pollLatch = new CountDownLatch(NUM_THREADS);
        List<Future<Integer>> futuresPolled = new ArrayList<>();
        for (int i = 0; i < NUM_THREADS; i++) {
            Future<Integer> result = threadPool.submit(() -> {
                int sum = 0;
                for (int i1 = 0; i1 < 10000; i1++) {
                    sum += queue.poll();
                }
                pollLatch.countDown();
                return sum;
            });
            futuresPolled.add(result);
        }

        try {
            pollLatch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // check sum
        int sumAdded = 0;
        int sumPolled = 0;
        try {
            for (Future<Integer> future : futuresAdded) {
                sumAdded += future.get();
            }
            for (Future<Integer> future : futuresPolled) {
                sumPolled += future.get();
            }
        } catch (InterruptedException | ExecutionException e) {
            e.printStackTrace();
        }

        assert (sumAdded == sumPolled);
    }
}
