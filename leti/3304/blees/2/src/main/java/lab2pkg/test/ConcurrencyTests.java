/*
 * Copyright (c) 2017, Red Hat Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Oracle nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
package lab2pkg.test;

import lab2pkg.source.LockFreePriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;
import org.openjdk.jcstress.infra.results.I_Result;

import static org.openjdk.jcstress.annotations.Expect.*;

// See jcstress-samples or existing tests for API introduction and testing guidelines

public class ConcurrencyTests {
    @State
    public static class QueueState {
        final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    }

    @JCStressTest
    @Outcome(id = "2", expect = ACCEPTABLE, desc = "Ok")
    @Outcome(id = "1", expect = FORBIDDEN, desc = "One lost")
    @Outcome(id = "0", expect = FORBIDDEN, desc = "Both lost")
    @Outcome(expect = FORBIDDEN)
    public static class AtomicityTest {
        @Actor
        public void actor1(QueueState st) {
            st.queue.offer(8);
        }

        @Actor
        public void actor2(QueueState st) {
            st.queue.offer(5);
        }

        @Arbiter
        public void arbiter1(QueueState st, I_Result result) {
            result.r1 = st.queue.size();
        }
    }

    @JCStressTest
    @Outcome(id = "-1, -1, -1", expect = FORBIDDEN, desc = "Two or all offers lost")
    @Outcome(id = "1, -1, -1", expect = FORBIDDEN, desc = "One or two offers lost")
    @Outcome(id = "2, -1, -1", expect = FORBIDDEN, desc = "One or two offers lost")
    @Outcome(id = "3, -1, -1", expect = FORBIDDEN, desc = "One or two offers lost")
    @Outcome(id = "5, 15, -1", expect = ACCEPTABLE, desc = "Ok")
    @Outcome(id = "5, 10, -1", expect = ACCEPTABLE, desc = "Ok")
    @Outcome(id = "10, 15, -1", expect = ACCEPTABLE, desc = "Ok")
    @Outcome(id = "5, 10, 15", expect = ACCEPTABLE, desc = "Ok")
    public static class ConsistencyTest {
        @Actor
        public void actor1(QueueState st) {
            st.queue.offer(5);
            st.queue.offer(10);
            st.queue.offer(15);
        }

        @Actor
        public void actor2(QueueState st) {
            st.queue.poll();
        }

        @Arbiter
        public void arbiter1(QueueState st, III_Result result) {
            Integer v1 = st.queue.poll();
            Integer v2 = st.queue.poll();
            Integer v3 = st.queue.poll();

            result.r1 = v1 != null ? v1 : -1;
            result.r2 = v2 != null ? v2 : -1;
            result.r3 = v3 != null ? v3 : -1;
        }
    }
}
