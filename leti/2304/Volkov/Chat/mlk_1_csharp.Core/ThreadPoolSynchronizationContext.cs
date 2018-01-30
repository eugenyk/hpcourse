using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace DevoidTalk.Core
{
    internal sealed class ThreadPoolSynchronizationContext : SynchronizationContext
    {
        private readonly ThreadPool threadPool;

        public ThreadPoolSynchronizationContext(ThreadPool threadPool)
        {
            this.threadPool = threadPool;
        }

        public override void Post(SendOrPostCallback callback, object state)
        {
            threadPool.Post(() => callback(state));
        }

        public override void Send(SendOrPostCallback callback, object state)
        {
            var resetEvent = new ManualResetEventSlim();
            threadPool.Post(() =>
            {
                callback(state);
                resetEvent.Set();
            });
            resetEvent.Wait();
        }

        public override SynchronizationContext CreateCopy()
        {
            return new ThreadPoolSynchronizationContext(threadPool);
        }
    }
}
