package ru.ifmo.ct.khalansky.hpcourse.queue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.*;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.*;

public class LinCheck {

    @StressCTest
    public static class LinCheckPriorityQueue {

        private PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

        @Operation
        public boolean offer(@Param(gen = IntGen.class, conf = "1:100")int key) {
            return queue.offer(key);
        }

        @Operation
        public Integer peek() {
            return queue.peek();
        }

        @Operation
        public Integer poll() {
            return queue.poll();
        }

        @Operation
        public boolean isEmpty() {
            return queue.isEmpty();
        }

    }

    @Test
    public void runTest() {
        LinChecker.check(LinCheckPriorityQueue.class);
    }

} 
