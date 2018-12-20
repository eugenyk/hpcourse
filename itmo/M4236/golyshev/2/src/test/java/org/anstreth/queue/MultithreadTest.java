package org.anstreth.queue;

import org.junit.Test;

import java.util.concurrent.*;
import java.util.function.BiFunction;

import static org.junit.Assert.assertEquals;

public class MultithreadTest {
    @Test
    public void joining_two_queues_is_correct() throws ExecutionException, InterruptedException {
        int N = 10000;
        int counterOne = 0;
        int counterTwo = 0;

        PriorityQueue<Integer> queueOne = new ConcurrentPriorityQueue<>();
        PriorityQueue<Integer> queueTwo = new ConcurrentPriorityQueue<>();
        PriorityQueue<Integer> expectedQueue = new ConcurrentPriorityQueue<>();
        PriorityQueue<Integer> resultQueue = new ConcurrentPriorityQueue<>();

        for (int i = 0; i < N; i++) {
            int value = ThreadLocalRandom.current().nextInt();
            boolean queueSelector = ThreadLocalRandom.current().nextBoolean();

            if (queueSelector) {
                counterOne++;
                queueOne.offer(value);
            } else {
                counterTwo++;
                queueTwo.offer(value);
            }

            expectedQueue.offer(value);
        }

        BiFunction<Integer, PriorityQueue<Integer>, Callable<Boolean>> taskCreator =
            (counter, queue) -> () -> {
                for (Integer i = 0; i < counter; i++) {
                    resultQueue.offer(queue.poll());
                }
                return true;
            };

        ExecutorService executorService = Executors.newFixedThreadPool(2);
        Future<Boolean> taskOne = executorService.submit(taskCreator.apply(counterOne, queueOne));
        Future<Boolean> taskTwo = executorService.submit(taskCreator.apply(counterTwo, queueTwo));

        taskOne.get();
        taskTwo.get();

        for (int i = 0; i < N; i++) {
            assertEquals(expectedQueue.poll(), resultQueue.poll());
        }

        executorService.shutdown();
    }
}
